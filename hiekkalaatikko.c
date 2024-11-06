char message[128]
uint8_t i = 0;
uint8_t spaces = 0;
while programState == INTERPRETING{
    if (spaces => 3){
        programState = MSG_SEND;
    }
    elif (DOT){
        message[i] = ".";
        spaces = 0;
        i++;
    }
    elif (DASH){
        message[i] = "-";
        spaces = 0;
        i++;
    }
    elif(SPACE){
        message[i] = " ";
        spaces++;
        i++;
    }
}
