from flask import Flask, render_template, request, redirect, url_for, session, jsonify, flash
import sqlite3
from functools import wraps
from werkzeug.security import generate_password_hash, check_password_hash
from datetime import datetime

app = Flask(__name__)
app.secret_key = "kl_hospital_secret_key_2026"
DATABASE = "hospital.db"

PRIORITY_NAMES = {4: "Critical", 3: "High", 2: "Medium", 1: "Low"}
SPECIALIZATIONS = [
    "General Medicine", "Cardiology", "Neurology", "Orthopedics",
    "General Surgery", "Pediatrics", "Emergency Medicine", "Pulmonology"
]
ROOM_TYPES = [
    "Operation Theatre", "ICU", "Monitoring Room",
    "General Room", "Private Room", "Emergency Observation"
]


def get_db():
    conn = sqlite3.connect(DATABASE)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    return conn


def table_columns(conn, table):
    return {row[1] for row in conn.execute(f"PRAGMA table_info({table})").fetchall()}


def add_column_if_missing(conn, table, column, definition):
    if column not in table_columns(conn, table):
        conn.execute(f"ALTER TABLE {table} ADD COLUMN {column} {definition}")


def init_database():
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS patients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_id TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            age INTEGER,
            gender TEXT,
            contact TEXT,
            medical_condition TEXT,
            arrival_time TEXT NOT NULL,
            priority INTEGER NOT NULL,
            status TEXT DEFAULT 'Waiting',
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    """)

    for column, definition in [
        ("scheduled_position", "INTEGER"),
        ("scheduled_at", "TEXT"),
        ("specialization", "TEXT DEFAULT 'General Medicine'"),
        ("treatment_type", "TEXT DEFAULT 'Consultation'"),
        ("room_required", "INTEGER DEFAULT 0"),
        ("room_type", "TEXT"),
        ("assigned_doctor_id", "INTEGER"),
        ("assigned_room_id", "INTEGER"),
        ("condition_updated_at", "TEXT"),
    ]:
        add_column_if_missing(conn, "patients", column, definition)

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL
        )
    """)

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS scheduling_runs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            run_time TEXT NOT NULL,
            total_patients INTEGER NOT NULL
        )
    """)

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS schedule_results (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            run_id INTEGER NOT NULL,
            patient_db_id INTEGER NOT NULL,
            position INTEGER NOT NULL,
            priority INTEGER NOT NULL,
            arrival_time TEXT NOT NULL,
            FOREIGN KEY (run_id) REFERENCES scheduling_runs(id),
            FOREIGN KEY (patient_db_id) REFERENCES patients(id)
        )
    """)

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS doctors (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            doctor_id TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            specialization TEXT NOT NULL,
            status TEXT DEFAULT 'Available',
            workload INTEGER DEFAULT 0,
            emergency_capable INTEGER DEFAULT 0
        )
    """)

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            room_number TEXT UNIQUE NOT NULL,
            room_type TEXT NOT NULL,
            status TEXT DEFAULT 'Available',
            current_patient_id INTEGER,
            FOREIGN KEY (current_patient_id) REFERENCES patients(id)
        )
    """)

    # Seed admin only if the table is empty.
    user = cursor.execute("SELECT id FROM users WHERE username = 'admin'").fetchone()
    if not user:
        cursor.execute(
            "INSERT INTO users (username, password) VALUES (?, ?)",
            ("admin", generate_password_hash("admin123"))
        )

    # Seed doctors only when none exist. Existing doctor data is preserved.
    if cursor.execute("SELECT COUNT(*) FROM doctors").fetchone()[0] == 0:
        doctors = [
            ("D001", "Dr. Arun Kumar", "Cardiology", "Available", 0, 1),
            ("D002", "Dr. Priya Sharma", "Neurology", "Available", 0, 1),
            ("D003", "Dr. Rahul Menon", "Orthopedics", "Available", 0, 0),
            ("D004", "Dr. Ananya Rao", "General Surgery", "Available", 0, 1),
            ("D005", "Dr. Neha Iyer", "Emergency Medicine", "Available", 0, 1),
            ("D006", "Dr. Vivek Das", "General Medicine", "Available", 0, 0),
        ]
        cursor.executemany("""
            INSERT INTO doctors
            (doctor_id, name, specialization, status, workload, emergency_capable)
            VALUES (?, ?, ?, ?, ?, ?)
        """, doctors)

    if cursor.execute("SELECT COUNT(*) FROM rooms").fetchone()[0] == 0:
        rooms = [
            ("OT-01", "Operation Theatre"),
            ("OT-02", "Operation Theatre"),
            ("ICU-01", "ICU"),
            ("ICU-02", "ICU"),
            ("MON-01", "Monitoring Room"),
            ("MON-02", "Monitoring Room"),
            ("GEN-01", "General Room"),
            ("GEN-02", "General Room"),
            ("PRI-01", "Private Room"),
            ("OBS-01", "Emergency Observation"),
        ]
        cursor.executemany(
            "INSERT INTO rooms (room_number, room_type) VALUES (?, ?)", rooms
        )

    conn.commit()
    conn.close()


def login_required(function):
    @wraps(function)
    def wrapper(*args, **kwargs):
        if "user_id" not in session:
            return redirect(url_for("login"))
        return function(*args, **kwargs)
    return wrapper


def priority_name(priority):
    return PRIORITY_NAMES.get(int(priority), "Low")


def infer_specialization(condition):
    text = (condition or "").lower()
    rules = [
        (("heart", "cardiac", "chest pain", "cardiology"), "Cardiology"),
        (("brain", "stroke", "seizure", "neurology"), "Neurology"),
        (("bone", "fracture", "joint", "orthopedic"), "Orthopedics"),
        (("surgery", "appendix", "appendicitis", "operation"), "General Surgery"),
        (("child", "pediatric", "paediatric"), "Pediatrics"),
        (("breath", "lung", "asthma", "pulmonary"), "Pulmonology"),
        (("emergency", "trauma", "accident"), "Emergency Medicine"),
    ]
    for keywords, spec in rules:
        if any(k in text for k in keywords):
            return spec
    return "General Medicine"


def infer_room(treatment_type, room_required, priority):
    if not room_required:
        return None
    text = (treatment_type or "").lower()
    if "surg" in text or "operation" in text:
        return "Operation Theatre"
    if "icu" in text or "intensive" in text:
        return "ICU"
    if "monitor" in text or "observation" in text:
        return "Monitoring Room" if priority < 4 else "Emergency Observation"
    if "private" in text:
        return "Private Room"
    return "General Room"


def choose_doctor(conn, patient):
    spec = patient["specialization"] or infer_specialization(patient["medical_condition"])
    query = """
        SELECT * FROM doctors
        WHERE status = 'Available' AND specialization = ?
        ORDER BY CASE WHEN ? = 4 THEN emergency_capable ELSE 0 END DESC,
                 workload ASC, id ASC
        LIMIT 1
    """
    doctor = conn.execute(query, (spec, patient["priority"])).fetchone()
    if doctor:
        return doctor

    # Emergency Medicine is a fallback for critical/emergency cases.
    if patient["priority"] == 4:
        return conn.execute("""
            SELECT * FROM doctors
            WHERE status = 'Available' AND emergency_capable = 1
            ORDER BY workload ASC, id ASC LIMIT 1
        """).fetchone()

    return conn.execute("""
        SELECT * FROM doctors
        WHERE status = 'Available'
        ORDER BY workload ASC, id ASC LIMIT 1
    """).fetchone()


def choose_room(conn, room_type):
    if not room_type:
        return None
    return conn.execute("""
        SELECT * FROM rooms
        WHERE status = 'Available' AND room_type = ?
        ORDER BY id ASC LIMIT 1
    """, (room_type,)).fetchone()


@app.route("/", methods=["GET", "POST"])
def login():
    if "user_id" in session:
        return redirect(url_for("dashboard"))
    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "")
        conn = get_db()
        user = conn.execute("SELECT * FROM users WHERE username = ?", (username,)).fetchone()
        conn.close()
        if user and check_password_hash(user["password"], password):
            session["user_id"] = user["id"]
            session["username"] = user["username"]
            return redirect(url_for("dashboard"))
        flash("Invalid username or password.", "error")
    return render_template("login.html")


@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("login"))


@app.route("/dashboard")
@login_required
def dashboard():
    conn = get_db()
    total_patients = conn.execute("SELECT COUNT(*) FROM patients").fetchone()[0]
    critical_patients = conn.execute("SELECT COUNT(*) FROM patients WHERE priority = 4 AND status = 'Waiting'").fetchone()[0]
    waiting_patients = conn.execute("SELECT COUNT(*) FROM patients WHERE status = 'Waiting'").fetchone()[0]
    completed_patients = conn.execute("SELECT COUNT(*) FROM patients WHERE status = 'Completed'").fetchone()[0]
    available_doctors = conn.execute("SELECT COUNT(*) FROM doctors WHERE status = 'Available'").fetchone()[0]
    available_rooms = conn.execute("SELECT COUNT(*) FROM rooms WHERE status = 'Available'").fetchone()[0]
    occupied_rooms = conn.execute("SELECT COUNT(*) FROM rooms WHERE status = 'Occupied'").fetchone()[0]
    recent_patients = conn.execute("SELECT * FROM patients ORDER BY id DESC LIMIT 5").fetchall()
    conn.close()
    return render_template("dashboard.html", total_patients=total_patients,
        critical_patients=critical_patients, waiting_patients=waiting_patients,
        completed_patients=completed_patients, available_doctors=available_doctors,
        available_rooms=available_rooms, occupied_rooms=occupied_rooms,
        recent_patients=recent_patients)


@app.route("/api/chart-data")
@login_required
def chart_data():
    conn = get_db()
    values = {}
    for number, name in [(4, "critical"), (3, "high"), (2, "medium"), (1, "low")]:
        values[name] = conn.execute(
            "SELECT COUNT(*) FROM patients WHERE priority = ?", (number,)
        ).fetchone()[0]
    conn.close()
    return jsonify(values)


@app.route("/patient-form", methods=["GET", "POST"])
@login_required
def patient_form():
    if request.method == "POST":
        patient_id = request.form.get("patient_id", "").strip()
        name = request.form.get("name", "").strip()
        age = request.form.get("age", "").strip()
        gender = request.form.get("gender", "").strip()
        contact = request.form.get("contact", "").strip()
        condition = request.form.get("medical_condition", "").strip()
        arrival_time = request.form.get("arrival_time", "").strip()
        priority = request.form.get("priority", "").strip()
        specialization = request.form.get("specialization", "") or infer_specialization(condition)
        treatment_type = request.form.get("treatment_type", "Consultation")
        room_required = 1 if request.form.get("room_required") else 0
        room_type = infer_room(treatment_type, room_required, int(priority or 1))

        if not patient_id or not name or not arrival_time or not priority:
            flash("Please fill all required fields.", "error")
            return render_template("patient_form.html", edit_mode=False)
        try:
            priority = int(priority)
            if priority not in [1, 2, 3, 4]:
                raise ValueError
            age_value = int(age) if age else None
        except ValueError:
            flash("Priority must be 1, 2, 3 or 4, and age must be numeric.", "error")
            return render_template("patient_form.html", edit_mode=False)

        conn = get_db()
        try:
            conn.execute("""
                INSERT INTO patients
                (patient_id, name, age, gender, contact, medical_condition,
                 arrival_time, priority, status, specialization, treatment_type,
                 room_required, room_type)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, 'Waiting', ?, ?, ?, ?)
            """, (patient_id, name, age_value, gender, contact, condition,
                  arrival_time, priority, specialization, treatment_type,
                  room_required, room_type))
            conn.commit()
            flash("Patient added successfully!", "success")
            return redirect(url_for("patients"))
        except sqlite3.IntegrityError:
            flash("Patient ID already exists.", "error")
        finally:
            conn.close()
    return render_template("patient_form.html", edit_mode=False)


@app.route("/patients")
@login_required
def patients():
    conn = get_db()
    patient_list = conn.execute("""
        SELECT p.*, d.doctor_id, d.name AS doctor_name,
               r.room_number, r.room_type AS assigned_room_type
        FROM patients p
        LEFT JOIN doctors d ON p.assigned_doctor_id = d.id
        LEFT JOIN rooms r ON p.assigned_room_id = r.id
        ORDER BY p.id DESC
    """).fetchall()
    conn.close()
    return render_template("patients.html", patients=patient_list)


@app.route("/edit-patient/<patient_id>", methods=["GET", "POST"])
@login_required
def edit_patient(patient_id):
    conn = get_db()
    patient = conn.execute("SELECT * FROM patients WHERE patient_id = ?", (patient_id,)).fetchone()
    if not patient:
        conn.close(); flash("Patient not found.", "error"); return redirect(url_for("patients"))

    if request.method == "POST":
        name = request.form.get("name", "").strip()
        age = request.form.get("age", "").strip()
        gender = request.form.get("gender", "").strip()
        contact = request.form.get("contact", "").strip()
        condition = request.form.get("medical_condition", "").strip()
        arrival_time = request.form.get("arrival_time", "").strip()
        priority = request.form.get("priority", "1").strip()
        specialization = request.form.get("specialization", "") or infer_specialization(condition)
        treatment_type = request.form.get("treatment_type", "Consultation")
        room_required = 1 if request.form.get("room_required") else 0
        try:
            priority = int(priority); age_value = int(age) if age else None
            if priority not in [1, 2, 3, 4]: raise ValueError
        except ValueError:
            conn.close(); flash("Invalid age or priority.", "error"); return redirect(url_for("edit_patient", patient_id=patient_id))
        room_type = infer_room(treatment_type, room_required, priority)
        conn.execute("""
            UPDATE patients SET name=?, age=?, gender=?, contact=?, medical_condition=?,
            arrival_time=?, priority=?, specialization=?, treatment_type=?, room_required=?, room_type=?,
            condition_updated_at=? WHERE patient_id=?
        """, (name, age_value, gender, contact, condition, arrival_time, priority,
              specialization, treatment_type, room_required, room_type,
              datetime.now().strftime("%Y-%m-%d %H:%M:%S"), patient_id))
        conn.commit(); conn.close()
        flash("Patient updated successfully. Queue will use the new priority.", "success")
        return redirect(url_for("patients"))
    conn.close()
    return render_template("patient_form.html", patient=patient, edit_mode=True)


@app.route("/delete-patient/<patient_id>", methods=["POST"])
@login_required
def delete_patient(patient_id):
    conn = get_db()
    patient = conn.execute("SELECT * FROM patients WHERE patient_id = ?", (patient_id,)).fetchone()
    if not patient:
        conn.close(); return jsonify({"success": False, "message": "Patient not found."})
    # Release resources first.
    if patient["assigned_room_id"]:
        conn.execute("UPDATE rooms SET status='Available', current_patient_id=NULL WHERE id=?", (patient["assigned_room_id"],))
    if patient["assigned_doctor_id"]:
        conn.execute("UPDATE doctors SET workload=MAX(workload-1,0) WHERE id=?", (patient["assigned_doctor_id"],))
    conn.execute("DELETE FROM patients WHERE patient_id = ?", (patient_id,))
    conn.commit(); conn.close()
    return jsonify({"success": True, "message": "Patient deleted successfully."})


@app.route("/queue")
@login_required
def queue():
    conn = get_db()
    waiting = conn.execute("""
        SELECT * FROM patients
        WHERE status = 'Waiting'
        ORDER BY priority DESC, arrival_time ASC, id ASC
    """).fetchall()
    conn.close()
    return render_template("queue.html", patients=waiting, priority_names=PRIORITY_NAMES)


@app.route("/emergency/<patient_id>", methods=["POST"])
@login_required
def emergency_patient(patient_id):
    conn = get_db()
    patient = conn.execute("SELECT * FROM patients WHERE patient_id=?", (patient_id,)).fetchone()
    if not patient:
        conn.close(); flash("Patient not found.", "error"); return redirect(url_for("patients"))
    conn.execute("""UPDATE patients SET priority=4, status='Waiting', condition_updated_at=? WHERE patient_id=?""",
                 (datetime.now().strftime("%Y-%m-%d %H:%M:%S"), patient_id))
    conn.commit(); conn.close()
    flash(f"{patient_id} marked Critical. Priority queue will place Critical patients first.", "success")
    return redirect(url_for("queue"))


@app.route("/update-priority/<patient_id>", methods=["POST"])
@login_required
def update_priority(patient_id):
    try:
        new_priority = int(request.form.get("priority", "1"))
        if new_priority not in [1, 2, 3, 4]: raise ValueError
    except ValueError:
        flash("Priority must be between 1 and 4.", "error"); return redirect(url_for("patients"))
    conn = get_db()
    conn.execute("UPDATE patients SET priority=?, status=CASE WHEN status='Completed' THEN status ELSE 'Waiting' END, condition_updated_at=? WHERE patient_id=?",
                 (new_priority, datetime.now().strftime("%Y-%m-%d %H:%M:%S"), patient_id))
    conn.commit(); conn.close()
    flash(f"Priority updated to {priority_name(new_priority)}.", "success")
    return redirect(url_for("queue"))


@app.route("/process")
@login_required
def process():
    conn = get_db()
    waiting = conn.execute("""
        SELECT * FROM patients WHERE status='Waiting'
        ORDER BY priority DESC, arrival_time ASC, id ASC
    """).fetchall()
    conn.close()
    steps = []
    for index, p in enumerate(waiting, 1):
        steps.append({
            "position": index, "patient_id": p["patient_id"], "name": p["name"],
            "priority": p["priority"], "priority_name": priority_name(p["priority"]),
            "arrival_time": p["arrival_time"], "specialization": p["specialization"] or infer_specialization(p["medical_condition"]),
            "room_type": p["room_type"] if p["room_required"] else "No room required"
        })
    return render_template("process.html", process_steps=steps, total=len(steps))


@app.route("/run-scheduling", methods=["POST"])
@login_required
def run_scheduling():
    conn = get_db()
    patients_to_schedule = conn.execute("""
        SELECT * FROM patients WHERE status='Waiting'
        ORDER BY priority DESC, arrival_time ASC, id ASC
    """).fetchall()
    if not patients_to_schedule:
        conn.close(); flash("There are no waiting patients to schedule.", "error"); return redirect(url_for("process"))

    run_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    run_cursor = conn.execute("INSERT INTO scheduling_runs (run_time,total_patients) VALUES (?,?)",
                              (run_time, len(patients_to_schedule)))
    run_id = run_cursor.lastrowid

    for position, patient in enumerate(patients_to_schedule, 1):
        doctor = choose_doctor(conn, patient)
        room_type = patient["room_type"] if patient["room_required"] else None
        room = choose_room(conn, room_type) if room_type else None

        # A patient can be scheduled only when all required resources are available.
        # Otherwise keep them Waiting so the next scheduling run can retry.
        doctor_id = doctor["id"] if doctor else None
        room_id = room["id"] if room else None
        resource_ready = doctor is not None and (not room_type or room is not None)

        conn.execute("""
            INSERT INTO schedule_results
            (run_id, patient_db_id, position, priority, arrival_time)
            VALUES (?, ?, ?, ?, ?)
        """, (run_id, patient["id"], position, patient["priority"], patient["arrival_time"]))

        if resource_ready:
            conn.execute("""
                UPDATE patients SET status='Scheduled', scheduled_position=?, scheduled_at=?,
                assigned_doctor_id=?, assigned_room_id=? WHERE id=?
            """, (position, run_time, doctor_id, room_id, patient["id"]))
            conn.execute("UPDATE doctors SET workload=workload+1 WHERE id=?", (doctor_id,))
            if room_id:
                conn.execute("UPDATE rooms SET status='Occupied', current_patient_id=? WHERE id=?", (patient["id"], room_id))

    conn.commit(); conn.close()
    flash("Priority scheduling completed. Doctor and room resources were allocated where available.", "success")
    return redirect(url_for("results"))


@app.route("/results")
@login_required
def results():
    conn = get_db()
    latest_run = conn.execute("SELECT * FROM scheduling_runs ORDER BY id DESC LIMIT 1").fetchone()
    result_list = []
    if latest_run:
        result_list = conn.execute("""
            SELECT sr.position, p.patient_id, p.name, p.age, p.gender,
                   p.medical_condition, sr.priority, sr.arrival_time, p.status,
                   p.specialization, p.treatment_type, p.room_required,
                   d.doctor_id, d.name AS doctor_name,
                   r.room_number, r.room_type
            FROM schedule_results sr
            JOIN patients p ON sr.patient_db_id=p.id
            LEFT JOIN doctors d ON p.assigned_doctor_id=d.id
            LEFT JOIN rooms r ON p.assigned_room_id=r.id
            WHERE sr.run_id=? ORDER BY sr.position ASC
        """, (latest_run["id"],)).fetchall()
    conn.close()
    return render_template("results.html", results=result_list, latest_run=latest_run)


@app.route("/doctors")
@login_required
def doctors():
    conn = get_db()
    doctor_list = conn.execute("SELECT * FROM doctors ORDER BY specialization, id").fetchall()
    conn.close()
    return render_template("doctors.html", doctors=doctor_list)


@app.route("/doctor-status/<doctor_id>", methods=["POST"])
@login_required
def doctor_status(doctor_id):
    status = request.form.get("status", "Available")
    if status not in ["Available", "Unavailable"]:
        status = "Available"
    conn = get_db()
    conn.execute("UPDATE doctors SET status=? WHERE doctor_id=?", (status, doctor_id))
    conn.commit(); conn.close()
    flash(f"Doctor {doctor_id} is now {status}.", "success")
    return redirect(url_for("doctors"))


@app.route("/rooms")
@login_required
def rooms():
    conn = get_db()
    room_list = conn.execute("""
        SELECT r.*, p.patient_id, p.name AS patient_name
        FROM rooms r LEFT JOIN patients p ON r.current_patient_id=p.id
        ORDER BY r.room_type, r.room_number
    """).fetchall()
    conn.close()
    return render_template("rooms.html", rooms=room_list)


@app.route("/room-status/<room_number>", methods=["POST"])
@login_required
def room_status(room_number):
    status = request.form.get("status", "Available")
    if status not in ["Available", "Maintenance"]:
        status = "Available"
    conn = get_db()
    room = conn.execute("SELECT * FROM rooms WHERE room_number=?", (room_number,)).fetchone()
    if room and room["status"] != "Occupied":
        conn.execute("UPDATE rooms SET status=? WHERE room_number=?", (status, room_number))
        conn.commit()
    conn.close()
    flash(f"Room {room_number} status updated.", "success")
    return redirect(url_for("rooms"))


@app.route("/allocation")
@login_required
def allocation():
    conn = get_db()
    assigned = conn.execute("""
        SELECT p.*, d.doctor_id, d.name AS doctor_name, d.specialization AS doctor_specialization,
               r.room_number, r.room_type
        FROM patients p
        LEFT JOIN doctors d ON p.assigned_doctor_id=d.id
        LEFT JOIN rooms r ON p.assigned_room_id=r.id
        WHERE p.status='Scheduled'
        ORDER BY p.priority DESC, p.scheduled_position ASC
    """).fetchall()
    conn.close()
    return render_template("allocation.html", patients=assigned)


@app.route("/discharge/<patient_id>", methods=["POST"])
@login_required
def discharge(patient_id):
    conn = get_db()
    patient = conn.execute("SELECT * FROM patients WHERE patient_id=?", (patient_id,)).fetchone()
    if not patient:
        conn.close(); flash("Patient not found.", "error"); return redirect(url_for("allocation"))
    if patient["assigned_room_id"]:
        conn.execute("UPDATE rooms SET status='Available', current_patient_id=NULL WHERE id=?", (patient["assigned_room_id"],))
    if patient["assigned_doctor_id"]:
        conn.execute("UPDATE doctors SET workload=MAX(workload-1,0) WHERE id=?", (patient["assigned_doctor_id"],))
    conn.execute("""
        UPDATE patients SET status='Completed', assigned_room_id=NULL, assigned_doctor_id=NULL WHERE patient_id=?
    """, (patient_id,))
    conn.commit(); conn.close()
    flash(f"{patient_id} discharged. Doctor and room resources are available again.", "success")
    return redirect(url_for("allocation"))


if __name__ == "__main__":
    init_database()
    print("\n==========================================")
    print("       KL HOSPITAL PRIORITY SYSTEM")
    print("==========================================")
    print("http://127.0.0.1:5000")
    print("Login: admin / admin123")
    print("==========================================\n")
    app.run(debug=True)
