const MORFO_database = "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/sensorData.json?apiKey=AIzaSyAf-6zUtowDnPI3FZbFEoT5HVJIzduKC00";
const sheet_name = 'TestSheet';

function fetchMORFOdata() {
    try {
        // HTTP request.
        const options = {
            method: 'get',
            contentType: 'application/json',
            muteHttpExceptions: true
        };

        const response = UrlFetchApp.fetch(MORFO_database, options);
        Logger.log("Response code: " + response.getResponseCode());

        const content = response.getContentText();
        Logger.log("Fetched content: " + content);

        if (!content) {
            throw new Error("Fetched content is null or empty.");
        }

        const morfo_data = JSON.parse(content);
        Logger.log("Parsed data: " + JSON.stringify(morfo_data));

        // Access the active spreadsheet and the sheet by name:
        const sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(sheet_name);

        // Check if sheet is empty, then add the header row:
        if (sheet.getLastRow() === 0) {
            sheet.appendRow(['Timestamp', 'EMG Value']);
        }

        // Append new data to the sheet:
        for (const key in morfo_data) {
            if (morfo_data.hasOwnProperty(key)) {
                const row = [
                    new Date(morfo_data[key].timestamp * 1000),
                    morfo_data[key].emgValue
                ];
                Logger.log("Row: " + row);
                sheet.appendRow(row);
            }
        }

        Logger.log("Data fetched and appended successfully.");
    } catch (error) {
        Logger.log("Error fetching or parsing data: " + error.message);
    }
}

