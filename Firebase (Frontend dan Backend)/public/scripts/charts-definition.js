// Create the charts when the web page loads
window.addEventListener("load", onload);

function onload(event) {
    chartT = createTemperatureChart();
    chartP = createPressureChart();
}

// Create Temperature Chart
function createTemperatureChart() {
    var chart = new Highcharts.Chart({
        chart: {
            renderTo: "chart-temperature",
            type: "spline",
        },
        series: [
            {
                name: "Kamar 1A",
            },
        ],
        title: {
            text: undefined,
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true,
                },
            },
        },
        xAxis: {
            type: "datetime",
            dateTimeLabelFormats: { second: "%H:%M:%S" },
        },
        yAxis: {
            title: {
                text: "Suhu (Celsius)",
            },
        },
        credits: {
            enabled: false,
        },
    });
    return chart;
}

// Create Pressure Chart
function createPressureChart() {
    var chart = new Highcharts.Chart({
        chart: {
            renderTo: "chart-pressure",
            type: "spline",
        },
        series: [
            {
                name: "Kamar 1A",
            },
        ],
        title: {
            text: undefined,
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true,
                },
            },
            series: {
                color: "#A62639",
            },
        },
        xAxis: {
            type: "datetime",
            dateTimeLabelFormats: { second: "%H:%M:%S" },
        },
        yAxis: {
            title: {
                text: "Kebisingan (dB)",
            },
        },
        credits: {
            enabled: false,
        },
    });
    return chart;
}
