const MORFO_database = "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/sensorData.json";
const sheet_name = 'TestSheet';

function fetchMORFOdata() {
    try {
        // HTTP request.
        const call = UrlFetchApp.fetch(MORFO_database);

        const content = call.getContentText();
        Logger.log("contenido obtenido: " + content);

        // response code logear (200)
        const responseCode = call.getResponseCode();
        Logger.log("Response Code: " + responseCode);

        // check if content is null or empty
        if (!content) {
            throw new Error("no data pipipi");
        }

        // Parsing the JSON:
        const morfo_data = JSON.parse(content);
        Logger.log("info parseada: " + JSON.stringify(morfo_data));

        // Use data from the named sheet:
        const sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(sheet_name);

        // Check if sheet is empty, then add the header row:
        if (sheet.getLastRow() === 0) {
            sheet.appendRow(['Timestamp', 'EMG Value']);
        }

        // Append new data:
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
    } catch (error) {
        Logger.log("Error: " + error.message);
    }
}

