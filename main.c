#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

typedef long long ll;

struct elements_for_frame {
    char ID[4], size[4], flags[2];
} headersize;

struct elements_for_main_header {
    char name[3] , countOfElement[4] , flag , size[4];
} insideF;

bool AreThatEquivalent(char* x, char* y);
int SizeOfCadre(char *pointsByte);
int EverythingCadres(FILE* input_File);
void setVal(FILE* fin, FILE* fout, char* someFrame, char* val);
void showFrame(FILE* f, char* frameID);
void setFrameSize(char* frameSize, int valSize);


int main(int argc, char* argv[]) {
    char textForParse[3][20];
    char argForParse[3][50];
    int l = 0;
    int m = 0;
    int r = 0;
    for (int i = 1; i < argc; i++) {
        l = 0;
        for (r = 0; argv[i][r] != '=' && argv[i][r] != '\0'; r++){
          textForParse[m][l++] = argv[i][r];
        }
        textForParse[m][l] = '\0';
        r += 1;
        l = 0;
        while (argv[i][r] != '\0'){
            argForParse[m][l++] = argv[i][r++];
        }
        argForParse[m][l] = '\0';
        m += 1;
    }
    FILE* inputAudio = fopen(argForParse[0], "r+b");
    FILE* outputAudio = fopen("output.mp3", "a+");
    if (inputAudio == NULL) {
        printf("(input)\nI cant open ur goddamn file..."); exit(1);
    }
    if (outputAudio == NULL){
        printf("(output)\nI cant open ur goddamn file..."); exit(1);
    }
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(textForParse[i], "--show") == 0) {
            EverythingCadres(inputAudio);
        }
        else if (strcmp(textForParse[i], "--get") == 0) {
            showFrame(inputAudio, argv[i]);
        }
        else if (strcmp(textForParse[i], "--set") == 0) {
            setVal(inputAudio, outputAudio, argv[i], argv[i + 1]);
        }
    }
    return 0;
}

bool AreThatEquivalent(char* x, char* y){
    for (int k = 0; k < sizeof(x); k++)
        if(x[k] != y[k]) return false;
    return true;
}

int SizeOfCadre(char *pointsByte) {
    int sch;
    sch = 0;
    unsigned char ch;
    for (int i = 0; i < 4; i++) {
        sch = pointsByte[i];
        sch += sch * pow(2, 7 * (3 - i));
    }
    return sch;
}

int EverythingCadres(FILE* input_File) {
    fread(&headersize, sizeof(headersize), 1, input_File);
    ll SizeOfThatHeader = SizeOfCadre(headersize.size);
    ll checker, CadreSizeSch;
    checker = 0;
    while(checker <= SizeOfThatHeader){
        fread(&insideF, sizeof(insideF), 1, input_File);
        CadreSizeSch = SizeOfCadre(insideF.size);
        checker += 10 + SizeOfCadre(insideF.size);
        if (insideF.name[0] == 'T'){
            printf("%s ", insideF.name);
            char equal;
            while(CadreSizeSch--){
                equal = fgetc(input_File);
                if (equal <= 126 && equal >= 32) printf("%c", equal);
            }
            printf(" \n ");
        }
        else
            if (AreThatEquivalent(insideF.name, "COMM") == true) {
                printf("%s ", insideF.name);
                fseek(input_File, 4, SEEK_CUR);
                char equal;
                CadreSizeSch = CadreSizeSch - 4;
                while(CadreSizeSch--) {
                    equal = fgetc(input_File);
                    if (equal <= 126 && equal >= 32) printf("%c", equal);
                }
                printf(" \n ");
            }
    }
}

void setVal(FILE* fin, FILE* fout, char* someFrame, char* val) {
    fseek(fin, 0, SEEK_SET);
    struct elements_for_main_header header;
    fread(&header, sizeof(header), 1, fin);
    fwrite(&header, sizeof(header), 1, fout);
    int valSize = strlen(val);
    int fileSize = SizeOfCadre(header.size);              ///filesize ==value sizze// total id3 tag size
    struct elements_for_main_header frame;
    int counter = 0;
    int frameSize = 0;
    while (counter <= fileSize) {
        fread(&frame, sizeof(frame), 1, fin);
        frameSize = SizeOfCadre(frame.size);
        counter += 10 + frameSize;
        if (AreThatEquivalent(frame.name, someFrame) == false && frameSize >= 0 && frameSize <= 100000) {
            fwrite(&frame, 1, sizeof(frame), fout);
            char info[frameSize];
            fread(&info, frameSize, 1, fin);
            fwrite(info, frameSize, 1, fout);
        }
        else if (AreThatEquivalent(frame.name, someFrame) == true && frameSize >= 0){
            int oldFrameSize = frameSize;
            setFrameSize(header.size, SizeOfCadre(header.size) - frameSize + valSize);
            int curPos = ftell(fout);
            fseek(fout, 6, SEEK_SET);
            fwrite(header.size, sizeof(header.size), 1, fout);
            fseek(fout, curPos, SEEK_SET);
            if (AreThatEquivalent(frame.name, "COMM") == true)
                setFrameSize(frame.size, valSize + 4);
            else if (AreThatEquivalent(frame.name, "TXXX") == true)
                setFrameSize(frame.size, valSize + 2);
            else
                setFrameSize(frame.size, valSize);
            int newFrameSize = SizeOfCadre(frame.size);
            fwrite(&frame, sizeof(frame), 1, fout);
            if (AreThatEquivalent(frame.name, "COMM") == true) {
                for (int i = 0; i < 4; i++) {
                    fputc(fgetc(fin), fout);
                }
                fseek(fin, oldFrameSize - 4, SEEK_CUR);
            }
            else if (AreThatEquivalent(frame.name, "TXXX") == true) {
                for (int i = 0; i < 2; i++) {
                    fputc(fgetc(fin), fout);
                }
                fseek(fin, oldFrameSize - 2, SEEK_CUR);
            }
            else
                fseek(fin, oldFrameSize, SEEK_CUR);
            fwrite(val, valSize, 1, fout);
        }
    }
    int ch;
    while ((ch = fgetc(fin)) != EOF) {
        fputc(ch, fout);
    }
    fseek(fout, 0, SEEK_SET);
    fseek(fin , 0, SEEK_SET);
    while ((ch = getc(fout)) != EOF) {
        fputc(ch, fin);
    }
}

void showFrame(FILE* f, char* frameID) {        /// we need to read the id3 header
    fseek(f, 0, SEEK_SET);
    struct elements_for_main_header header;
    fread(&header, sizeof(header), 1, f);
    int fileSize = SizeOfCadre(header.size);
    struct elements_for_frame frame;
    int counter = 0;
    while (counter <= fileSize) {
        fread(&frame, sizeof(frame), 1, f);
        int frameSize = SizeOfCadre(frame.size);
        counter += 10 + frameSize;    /// because the header is 10 bytes
        if (AreThatEquivalent(frame.ID, frameID) == true) {
            printf("%s ", frame.ID);
            while(frameSize--) {
                char ch = fgetc(f);
                if (ch >= 32 && ch <= 126)
                    printf("%c", ch);
            }
        }
        else {
            fseek(f, SizeOfCadre(frame.size), SEEK_CUR);
        }
    }
}

void setFrameSize(char* frameSize, int valSize) {
    for (int i = 0; i < 4; i++) {
        frameSize[i] = valSize / pow(2, 7 * (3 - i));
        valSize -= frameSize[i] * pow(2, 7 * (3 - i));
    }
}