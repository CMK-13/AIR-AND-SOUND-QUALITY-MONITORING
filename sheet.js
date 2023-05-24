// https://script.google.com/macros/s/AKfycbzeCOu0KsbojwLJQsTUyqodIWXx0tBi5chuZcwBlgGFzWJ9PfivL0ASPeUflkI_-DoMew/exec
function doGet(e) {
    Logger.log(JSON.stringify(e));
    var result = 'Ok';
    if (e.parameter == 'undefined') {
        result = 'No Parameters';
    }
    else {
        var sheet_id = '1RzOs-Z1f2hzvTQPXEAwBvMChnmBgctvFOcbjbgt2A50'; 	// Spreadsheet ID
        var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
        var newRow = sheet.getLastRow() + 1;
        var rowData = [];
        var Curr_Date = new Date();
        rowData[0] = Curr_Date; // Date in column A
        var Curr_Time = Utilities.formatDate(Curr_Date, "Asia/Jakarta", 'HH:mm:ss');
        rowData[1] = Curr_Time; // Time in column B
        for (var param in e.parameter) {
            Logger.log('In for loop, param=' + param);
            var value = stripQuotes(e.parameter[param]);
            Logger.log(param + ':' + e.parameter[param]);
            if (param == 'amthanh') {
                rowData[2] = value; // data in column C
            }
            if (param == 'khongkhi') {
                rowData[3] = value; // data in column d
            }
        }
        Logger.log(JSON.stringify(rowData));
        var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
        newRange.setValues([rowData]);
    }
    return ContentService.createTextOutput(result);
}
function stripQuotes(value) {
    return value.replace(/^["']|['"]$/g, "");
}



function updateColumnE() {
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    var dataRange = sheet.getRange("C2:C"); // Ph?m vi d? li?u trong c?t C, b?t ??u t? hàng th? 2
    var data = dataRange.getValues(); // L?y giá tr? d? li?u trong c?t C
    var columnERange = sheet.getRange("E2:E"); // Ph?m vi d? li?u trong c?t E, b?t ??u t? hàng th? 2
    var columnEValues = columnERange.getValues(); // L?y giá tr? d? li?u trong c?t E

    for (var i = 0; i < data.length; i++) {
        if (columnEValues[i][0] === "") { // Ki?m tra n?u ô trong c?t E ch?a có giá tr?
            columnEValues[i][0] = i + 1; // Gán giá tr? s? th? t? vào ô t??ng ?ng trong c?t E
        }
    }

    columnERange.setValues(columnEValues); // C?p nh?t giá tr? vào c?t E
}

function scheduleUpdateColumnE() {
    var frequencyInSeconds = 5; // T?n su?t th?c hi?n c?p nh?t, ví d?: 5 giây

    // ??t l?ch trình th?c thi ch??ng trình updateColumnE() theo t?n su?t
    ScriptApp.newTrigger("updateColumnE")
        .timeBased()
        .everySeconds(frequencyInSeconds)
        .create();
}




// JavaScript source code
