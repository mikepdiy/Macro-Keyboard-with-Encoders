void sdFileToKeyboard1() {
    File dataFile = fatfs.open(filenameOnCard1);
    if (!dataFile) {
      Serial.println("The specified filename is not present on the memory ic, check filenameOnCard !");
    }
    String line;
    while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        Serial.println(line);
        sendToKeyboard(line);
    }
    dataFile.close();
}

void sdFileToKeyboard2() {
    File dataFile = fatfs.open(filenameOnCard2);
    if (!dataFile) {
      Serial.println("The specified filename is not present on the memory ic, check filenameOnCard !");
    }
    String line;
    while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        Serial.println(line);
        sendToKeyboard(line);
    }
    dataFile.close();
}

void sdFileToKeyboard3() {
    File dataFile = fatfs.open(filenameOnCard3);
    if (!dataFile) {
      Serial.println("The specified filename is not present on the memory ic, check filenameOnCard !");
    }
    String line;
    while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        Serial.println(line);
        sendToKeyboard(line);
    }
    dataFile.close();
}

void sdFileToKeyboard4() {
    File dataFile = fatfs.open(filenameOnCard4);
    if (!dataFile) {
      Serial.println("The specified filename is not present on the memory ic, check filenameOnCard !");
    }
    String line;
    while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        Serial.println(line);
        sendToKeyboard(line);
    }
    dataFile.close();
}

void sdFileToKeyboard5() {
    File dataFile = fatfs.open(filenameOnCard5);
    if (!dataFile) {
      Serial.println("The specified filename is not present on the memory ic, check filenameOnCard !");
    }
    String line;
    while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        Serial.println(line);
        sendToKeyboard(line);
    }
    dataFile.close();
}

void sdFileToKeyboard6() {
    File dataFile = fatfs.open(filenameOnCard6);
    if (!dataFile) {
      Serial.println("The specified filename is not present on the memory ic, check filenameOnCard !");
    }
    String line;
    while (dataFile.available()) {
        line = dataFile.readStringUntil('\n');
        Serial.println(line);
        sendToKeyboard(line);
    }
    dataFile.close();
}

void sendToKeyboard(String line) {
    String workingLine = line;
    if (workingLine.indexOf(sleepCommandStartingPoint) != -1) {
        sleepFor(line);
        return;      
    }
    if (workingLine.indexOf(commandStartingPoint) == -1) {
        Keyboard.print(line);
 
        
        return;        
    }    

    int charPosition = commandStartingPoint.length();
    int lineLength = line.length();
    workingLine += ",";

    
    while (workingLine != "") {
        workingLine = workingLine.substring(charPosition);
        int specialCommandDelimiterPosition = workingLine.indexOf(",");
        String command = workingLine.substring(0, specialCommandDelimiterPosition);
        charPosition = specialCommandDelimiterPosition + 1;
        if (command.startsWith("KEY", 0)) {
            Keyboard.press(getCommandCode(command));
            delay(delayBetweenCommands);
        }
        if (command.startsWith("MOUSE", 0)) {
            Mouse.press(getCommandCode(command));
            delay(delayBetweenCommands);
        }
    }


}

void pressEnter() {
    Keyboard.press(KEY_RETURN);
    Keyboard.releaseAll();
}

void sleepFor(String line) {
    int sleepAmount = line.substring(sleepCommandStartingPoint.length(), line.length()).toInt();
    Serial.print("Sleeping for:");Serial.println(sleepAmount);
    delay(sleepAmount);
}

char getCommandCode(String text) {
    char textCharacters[2]; 
    text.toCharArray(textCharacters, 2);
    char code = textCharacters[0];
    
    code = (text == "KEY_LEFT_CTRL") ? KEY_LEFT_CTRL : code;
    code = (text == "KEY_CMD") ? KEY_LEFT_ALT : code;
    code = (text == "KEY_LEFT_GUI") ? KEY_LEFT_GUI : code;
    code = (text == "KEY_OPT") ? KEY_LEFT_GUI : code;
    code = (text == "KEY_LEFT_SHIFT") ? KEY_LEFT_SHIFT : code;
    code = (text == "KEY_LEFT_ALT") ? KEY_LEFT_ALT : code;
    code = (text == "KEY_UP_ARROW") ? KEY_UP_ARROW : code;
    code = (text == "KEY_DOWN_ARROW") ? KEY_DOWN_ARROW : code;
    code = (text == "KEY_LEFT_ARROW") ? KEY_LEFT_ARROW : code;
    code = (text == "KEY_RIGHT_ARROW") ? KEY_RIGHT_ARROW : code;
    code = (text == "KEY_RIGHT_GUI") ? KEY_RIGHT_GUI : code;
    code = (text == "KEY_BACKSPACE") ? KEY_BACKSPACE : code;
    code = (text == "KEY_TAB") ? KEY_TAB : code;
    code = (text == "KEY_RETURN") ? KEY_RETURN : code;
    code = (text == "KEY_ESC") ? KEY_ESC : code;
    code = (text == "KEY_INSERT") ? KEY_INSERT : code;
    code = (text == "KEY_DELETE") ? KEY_DELETE : code;
    code = (text == "KEY_PAGE_UP") ? KEY_PAGE_UP : code;
    code = (text == "KEY_PAGE_DOWN") ? KEY_PAGE_DOWN : code;
    code = (text == "KEY_HOME") ? KEY_HOME : code;
    code = (text == "KEY_END") ? KEY_END : code;
    code = (text == "KEY_CAPS_LOCK") ? KEY_CAPS_LOCK : code;
    code = (text == "KEY_F1") ? KEY_F1 : code;
    code = (text == "KEY_F2") ? KEY_F2 : code;
    code = (text == "KEY_F3") ? KEY_F3 : code;
    code = (text == "KEY_F4") ? KEY_F4 : code;
    code = (text == "KEY_F5") ? KEY_F5 : code;
    code = (text == "KEY_F6") ? KEY_F6 : code;
    code = (text == "KEY_F7") ? KEY_F7 : code;
    code = (text == "KEY_F8") ? KEY_F8 : code;
    code = (text == "KEY_F9") ? KEY_F9 : code;
    code = (text == "KEY_F10") ? KEY_F10 : code;
    code = (text == "KEY_F11") ? KEY_F1 : code;
    code = (text == "KEY_F12") ? KEY_F2 : code;
    code = (text == "MOUSE_LEFT") ? MOUSE_LEFT : code;
    code = (text == "MOUSE_RIGHT") ? MOUSE_RIGHT : code;
    code = (text == "MOUSE_MIDDLE") ? MOUSE_MIDDLE : code;
    

    return code;
}
