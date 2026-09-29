// =====================================================
// KL HOSPITAL DASHBOARD
// PRIORITY DISTRIBUTION CHART
// =====================================================

document.addEventListener("DOMContentLoaded", function () {

    const chartCanvas = document.getElementById("priorityChart");

    if (!chartCanvas) {
        return;
    }


    // Get patient priority information from Flask
    fetch("/api/chart-data")
        .then(response => response.json())
        .then(data => {

            new Chart(chartCanvas, {

                type: "bar",

                data: {

                    labels: [
                        "Critical",
                        "High",
                        "Medium",
                        "Low"
                    ],

                    datasets: [

                        {
                            label: "Patients",

                            data: [
                                data.critical,
                                data.high,
                                data.medium,
                                data.low
                            ],

                            borderWidth: 0,

                            borderRadius: 7

                        }

                    ]

                },

                options: {

                    responsive: true,

                    maintainAspectRatio: false,

                    plugins: {

                        legend: {
                            display: false
                        }

                    },

                    scales: {

                        y: {

                            beginAtZero: true,

                            ticks: {
                                precision: 0
                            }

                        },

                        x: {

                            grid: {
                                display: false
                            }

                        }

                    }

                }

            });

        })

        .catch(error => {

            console.error(
                "Unable to load chart data:",
                error
            );

        });

});