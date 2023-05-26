const functions = require("firebase-functions");
var nodemailer = require("nodemailer");
const fs = require("fs");
var handlebars = require("handlebars");

// // Create and deploy your first functions
// // https://firebase.google.com/docs/functions/get-started

var readHTMLFile = function (path, callback) {
    fs.readFile(path, { encoding: "utf-8" }, function (err, html) {
        if (err) {
            callback(err);
        } else {
            callback(null, html);
        }
    });
};

exports.emailNotification = functions.database
    .ref("UsersData/{espID}/readings/{readingId}")
    .onCreate((snapshot, context) => {
        console.log("masuuuuk functionnya");
        dataSensor = snapshot.val();
        console.log(dataSensor);
        console.log("Ini temperatur");
        console.log(dataSensor.temperature);
        var temperature = dataSensor.temperature;
        var kebisingan = dataSensor.pressure;
        var kamar = dataSensor.kamar;
        var textEmail = `Suhu atau kebisingan terdeteksi melebihi batas yang diperbolehkan dengan detail sebagai berikut:\n\nKamar: ${kamar} \nSuhu: ${temperature}°C  \nTingkat kebisingan: ${kebisingan} dB`;
        console.log(textEmail);

        if (dataSensor.isExceedTemp == "True" || dataSensor.isExceedNoise == "True") {
            var transporter = nodemailer.createTransport({
                service: "gmail",
                auth: {
                    user: "kofifanhertza@gmail.com",
                    pass: "quawdcugjnnwyywq",
                },
            });
            readHTMLFile("index.html", function (err, html) {
                if (err) {
                    console.log("error reading file", err);
                    return;
                }
                var template = handlebars.compile(html);
                var replacements = {
                    kamar: kamar,
                    temperature: temperature,
                    kebisingan: kebisingan,
                };
                var htmlToSend = template(replacements);
                var mailOptions = {
                    from: "kofifanhertza@gmail.com",
                    // to: "18220042@std.stei.itb.ac.id, 18220022@std.stei.itb.ac.id, 18220082@std.stei.itb.ac.id",
                    to: "18220042@std.stei.itb.ac.id",
                    subject:
                        "Peringatan euy! Suhu atau Kebisingan Melebihi Batas yang Diperbolehkan",
                    html: htmlToSend,
                };
                transporter.sendMail(mailOptions, (err, info) => {
                    if (err) throw err;
                    console.log("Email sent: " + info.response);
                });
            });
        }
    });
