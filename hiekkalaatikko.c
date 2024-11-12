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

enum nuotisto{
C3 = 131,
Cis3 = 139,
D3 = 147,
Dis3 = 156,
E3 = 165,
F3 = 175,
Fis3 = 185,
G3 = 196,
Gis3 = 208,
A3 = 220,
Bes3 = 233,
B3 = 247,
C4 = 262,
Cis4 = 277,
D4 = 294,
Dis4 = 311,
E4 = 330,
F4 = 349,
Fis4 = 370,
G4 = 392,
Gis4 = 415,
A4 = 440,
Bes4 = 466,
B4 = 494,
C5 =  523,
Cis5 = 554,
D5 = 587,
Dis5 = 622,
E5 = 659,
F5 = 698,
Fis5 =740,
G5 = 784,
Gis5 = 831,
A5 = 880,
Bes5 = 932,
B5 = 988
}

void note(buzzer, freq, time){
    buzzerOpen(buzzer);
    buzzerSetFrequency(freq);
    Task_sleep(time);
    buzzerClose();

}