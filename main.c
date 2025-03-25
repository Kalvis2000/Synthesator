
//Created by Kalvi Teldre GPL3 licence!!!

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL3\SDL.h>
//#include <SDL3\SDL_main.h>
#include <SDL3\SDL_events.h>






short volu=0; //sama mis static


#define MAXPROGRAMS 8
#define MAXGENERATORS 32
#define MAXCHORDS 8;
#define MAXPARAMINDEXS 16
#define MAXPARAMS 255

typedef struct {
char *name;//for DB

//unsigned char FiltFlaf;//Filter settings flag
unsigned char octav;//default octav for instrument.
//unsigned char Volume;// general output volume
unsigned char Vol[6];
unsigned short flags[6];// !2b FHP!2b FLP!*LPmod!*HPmod!*Mixp!*Ringp!     !*dUp!E!S!C!4b wave!
unsigned short PulseLevel;
// saw,triangle,pulse,sinus,noise,ring,pulsemodul,
float freq[6]; //generators freqrange = dUfreq
float dUpfreq; //if used freq modul, then rate constant
float CutOfffreqLP;
float CutOfffreqHP;
float CutOfffreqLPmodR;
float CutOfffreqHPmodR;

//ADSR
unsigned short adsr[8];//adsr 2 compleckt, time in ms

char *params;//all additional parameters

} SynthInstr_t;


SynthInstr_t singlegen={"singlegen",4,
100,0,0,0,0,0,//vol
0x0020,0,0,0,0,0,//flags
0x8000,
1,0,0,0,0,0//freq
,0,0,0,0,0,
//adsr
10,300,0x8000,300,1,1,1,1,0};

struct MainSynth {
//General parameters
float ScardT;//=1/Scardf
float LFO; //7 Hz LFO generator for vibratio or tremolo
unsigned short Scardf;
unsigned char ChannelsType;//mono, stereo, 5.1, 7.1 etc
unsigned char Programs;
unsigned char Generators;

//Program parameters
char *PRG[MAXPROGRAMS];//program stream (note) pointer
char *PCM[MAXPROGRAMS];//if used PCM, bezier, polynom or furie waves, then pointer wave data
unsigned short IP[MAXPROGRAMS];//Program instruction counter
//unsigned int NoteT[MAXPROGRAMS];//Noodi dT tactide arv?
unsigned char firsGtrack[MAXPROGRAMS];//
char crescendo[MAXPROGRAMS];//volume +- every note!
unsigned short PulseLevel[MAXPROGRAMS];//Võib minna track põhiseks
float dU[MAXPROGRAMS];//Note frequency!  there only 5bit for chords to point chords dU
float MixVolOut[MAXPROGRAMS]; //Hoiab Program põhist heli väljundit.

//Filters
float CutOffHP[MAXPROGRAMS];
float CutOffLP[MAXPROGRAMS];
float CutoffHPrate[MAXPROGRAMS];
float CutoffLPrate[MAXPROGRAMS];
float *CutOffHPmodp[MAXPROGRAMS];
float *CutOffLPmodp[MAXPROGRAMS];
//Envelopes
unsigned char Estage[MAXPROGRAMS];
unsigned char Estagef[MAXPROGRAMS];
float Ereg[MAXPROGRAMS];//0 - volume, 1-filters,2 - static - typical is 0,1 or range const
float Eregf[MAXPROGRAMS];
float a[MAXPROGRAMS][5];//parabooli tõusukõverad - vol
float af[MAXPROGRAMS][5];//filter parabole
unsigned int n[MAXPROGRAMS][9];//et ei peaks koguaeg ajakonstante arbutama
unsigned int nxv[MAXPROGRAMS][5];//hoiab vahetulemust
unsigned int nxf[MAXPROGRAMS][5];//hoiab vahetulemust
//Interrupts signals - char - 8 int 32
unsigned char SSync; //SoftSyncis ei ole maski - iga track vaatab oma bitti
unsigned char SSyncmask;//????
unsigned char Test; //Genrator vaatab kah oma test bitti tööks
unsigned char HSync;//genreerib wav flag
unsigned char vibrato;
unsigned char tremolo;
unsigned char HSyncmaskprg[MAXPROGRAMS]; //program sync mask
unsigned int HSyncmasktr;//track põhine mask mis luab aktiivseid trvcke syncida (max 32 track is allowed)
//Program settings
unsigned char repeats[MAXPROGRAMS];
float *dUp[MAXPROGRAMS]; //only 1 freq or pulse modulators per program
float dUpfreq[MAXPROGRAMS];//Used freq or Pulse modulation freg correction rate? Võib minna ka track põhiseks, veel ei tea
float *Mixp[MAXPROGRAMS]; //LFO Volume modulation pointer in track volume output (pointed 0...0xFFFF volume register, typical GenV output (LFO)
float *Ringp[MAXPROGRAMS]; //Ringmodulation pointer. used also other modulation. Typical pointed other track GenV or adsr volume
float FOutHP[MAXPROGRAMS];//Filter HP output (also used for read old value)
float FInoldHP[MAXPROGRAMS];//Filter old input (default it is MixVolOut, but must old)
float FOutLP[MAXPROGRAMS];//Filter Output (also used for read old value)

unsigned char MixPV[MAXPROGRAMS]; //General last volume 0...100 max 255. Program volume
unsigned char Octav[MAXPROGRAMS];
unsigned char bpm[MAXPROGRAMS];
unsigned char chords[MAXPROGRAMS];//chords
unsigned char choind[MAXPROGRAMS];//chorrd memori indeks

//Generators
unsigned char Gprg[MAXGENERATORS]; //param index??|4 bit progr nr.
unsigned char parami[MAXGENERATORS];//tr
unsigned char wavef[MAXGENERATORS];// |Const|SYNC|Ereg|4b wave|??

unsigned char MixVol[MAXGENERATORS];//track const Volume 0..100, max 255
unsigned char MixpV[MAXGENERATORS];//Mixp Volume correction
unsigned short Saw[MAXGENERATORS];//

float dUfreq[MAXGENERATORS];
float GenV[MAXGENERATORS];

short paramindex[16]; //max 32 indexes!!! more need gener|tor extr| v|ri|ble
unsigned char param[256];
} Msynth;

//char harm[11]={100,77,18,11,5,2,0,0,0,0,0};
unsigned char pcmharn[256];


int FurieToPCM(unsigned char *pcm,char *Harmonics,unsigned char harmcounter){
//This function converts furie getted harmonics amplitudes to wave
float dt=M_PI/128;
float f=0,m=255;
float t=0;
unsigned char x,i;
int ha=0;
for (i=0;i<harmcounter;i++) ha +=*(Harmonics+i)&0x7F;//get absolute max. amplitudes
m /=ha;
printf("dt=%f\n",dt);
for (x=0;x<256;x++) {
    f=0;
    for (i=0;i<harmcounter;i++) f +=*(Harmonics+i)*sin((i+1)*t)*m;
    if (f>255) return -1;
    *pcm=(int)f;
    t +=dt;
    pcm++;
}
return 0;
}

float onef=1;
int Sprocessor(unsigned char prg){
unsigned char *PC;
unsigned short usv=0;
PC=(unsigned char*) Msynth.PRG[prg];
unsigned char mark=*(PC+Msynth.IP[prg]);
unsigned char testbit=1<<prg;
int iv=0;
float fv=0;
Msynth.IP[prg]++;

switch(mark){
case 0: //end
    Msynth.IP[prg]--;
    Msynth.Test &=!(1<<prg);
    return 0;
case 'a'://Note a
    mark=*(PC+Msynth.IP[prg]);
    fv=27.5; //Note a0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// a- G+
        fv=25.9565;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//a+
        fv=29.1352; //=B-
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("a-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'b'://Note b
    mark=*(PC+Msynth.IP[prg]);
    fv=30.8677; //Note B0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// -
        fv=29.1352;//=a+
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//+ Not exist
        fv=31.7722; //+50 cents!
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("B-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'c'://Note C
    mark=*(PC+Msynth.IP[prg]);
    fv=16.3516; //Note C0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// C-
        fv=15.8861;// -50 cents ?B-1=15.4339
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//C+
        fv=17.3239; //D-
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("C-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'd'://Note D
    mark=*(PC+Msynth.IP[prg]);
    fv=18.354; //Note D0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// D-
        fv=17.3239;//=C+
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//D+
        fv=19.4454; //=E-
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("D-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'e'://Note E
    mark=*(PC+Msynth.IP[prg]);
    fv=20.6017; //Note E0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// E-
        fv=19.4454;//=D+
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//E+ not exist
        fv=21,2054; //=F- +50 cents E
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("E-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'f'://Note F
    mark=*(PC+Msynth.IP[prg]);
    fv=21.8268; //Note F0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// F- not exist
        fv=21.2054;//=E+
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//F+
        fv=23.1247; //=G-
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("F-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'g'://Note G
    mark=*(PC+Msynth.IP[prg]);
    fv=24.4997; //Note G0
    if (mark=='.'){
        usv=1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==45) {// G-
        fv=23.1247;//=F+
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//G+
        fv=25.9565; //=a1-
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //note octav
    fv *=1<<Msynth.Octav[prg];
    Msynth.dU[prg]=fv*0xFFFF*Msynth.ScardT;
    printf("G-dU=%f,",Msynth.dU[prg]);
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    Msynth.n[prg][2] +=usv*.5*Msynth.n[prg][2];
    //Sustain time calculated in Envelopes!
    printf("Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=5;
    return 0;
case 'p': //silence!
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    //Sustain time calculated in Envelopes!
    printf("Silence Tickse=%d,",Msynth.n[prg][2]);
    Msynth.Estage[prg]=6;
    return 0;

case 'o': //Octav 04, o+,o-
    mark=*(PC+Msynth.IP[prg]);
    //fv=0;
    if (mark==45) {// -
        fv=-1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//+
        fv=1; //
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (fv!=0) iv=Msynth.Octav[prg]+fv*iv;//suhteline + -
    if (iv<0 || iv>11) iv=4;
    Msynth.Octav[prg]=iv;
    printf("octav=%d,",Msynth.Octav[prg]);
    return 0;
case 't': //tempo. beats per minute
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv<40) iv=400;
    if (iv>255) iv=120;
    Msynth.bpm[prg]=iv;
    return 0;
case 'V': //Vibrato
    Msynth.vibrato ^=testbit;
    return 0;
case 'T'://Tremolo - Volume LFO modulation
    Msynth.tremolo ^=testbit;
    return 0;
case 'v': //Volume
    mark=*(PC+Msynth.IP[prg]);
    fv=0;
    if (mark==45) {// -
        fv=-1;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    if (mark==43) {//+
        fv=1; //
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }

    if (fv!=0) iv=Msynth.MixPV[prg]+fv*iv;
    if (iv<0) iv=0;
    if (iv>255) iv=255;
    printf("Program Vol=%d",iv);
    Msynth.MixPV[prg]=iv;
    break;
case 'W': //soft sync wait
    Msynth.Test &=(1<<prg); //stop program (genervtors)
    //no mask, SSync bit starts
    return 0;
case 'S': //Syncronise - väga lihtne
    mark=*(PC+Msynth.IP[prg])-48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>255) iv=255;
    Msynth.SSync |=iv;
    return 0;

case 'H': //Highpass Filter settings H+- set ON/OFF or cutoff frequency
    if (mark==45) {// -
        Msynth.wavef[prg] &=0x7F;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
        usv=1;
    }
    if (mark==43) {//+
        Msynth.wavef[prg] |=128;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
        usv=1;
    }
    mark -=48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 && usv==0) {//
        Msynth.wavef[prg]=(!Msynth.wavef[prg])&0x800+Msynth.wavef[prg]&0x7F;
        return 0;
    }
    //cutoff freq
    Msynth.CutOffHP[prg]=iv;
    printf("NewF H CutOff=%d,",iv);
    return 0;
case '<': //crescendo go higher
    if (Msynth.crescendo[prg]<0) { //if negative, then exit
        Msynth.crescendo[prg]=0;
        return 0;
    }
    mark=*(PC+Msynth.IP[prg])-48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>9) iv=1;
    Msynth.crescendo[prg]=iv;
    return 0;
case '>': //crescendo go lower
    if (Msynth.crescendo[prg]>0) { //if positive, then exit
        Msynth.crescendo[prg]=0;
        return 0;
    }
    mark=*(PC+Msynth.IP[prg])-48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>9) iv=1;
    Msynth.crescendo[prg]=-iv;
    return 0;
case '[': //repeats start
    mark=Msynth.paramindex[0];//uus ineks
    if (mark==MAXPARAMINDEXS) return -2;//no free indeks
    usv=Msynth.paramindex[mark];//uus mäluviit
    Msynth.paramindex[0]++;
    Msynth.param[usv]=0; //rep =0 if ] then right vlue
    Msynth.param[usv+1]=Msynth.repeats[prg];//eelmine indeks hoiule, kui on 0 siis on viimane
    printf("Rold=%d,IPoff=%d,",Msynth.repeats[prg],Msynth.IP[prg]);
    *(unsigned short*)(Msynth.param+usv+2)=Msynth.IP[prg]; //see on kenasti üle [
    Msynth.repeats[prg]=mark; //repeatsi
    mark++;
    usv +=4;
    if (usv>MAXPARAMS) return -3;    //no free memory
    Msynth.paramindex[mark]=usv;
    return 0;
case ']': //repeats END!
    mark=*(PC+Msynth.IP[prg])-48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv>=255 || iv==0) iv=255; //infinity
    //read dyn mem repeats ? see peaks short olemv
    mark=Msynth.repeats[prg];
    if (!mark) return -5; //repeats index not may 0
    usv=Msynth.paramindex[mark];//lei
    mark=Msynth.param[usv];//read repeats value
    //infinity
    if (mark==255 || iv==255){//infinity!
        Msynth.param[usv]=255;
        Msynth.IP[prg]=*(unsigned short*)(Msynth.param+usv+2);
        return 0;
    }
    //no infinity, repeats to zero - kui on suurem null siis on repeat töös
    if (Msynth.param[usv]) {//if not zero, them -1
        Msynth.param[usv]--;
        printf("Repeats=%d,",Msynth.param[usv]);
        if (Msynth.param[usv]==0){//END repeats
            Msynth.paramindex[Msynth.repeats[prg]] *=-1;
            Msynth.repeats[prg]=Msynth.param[usv+1]; //set to old repeats
            //memory free if possible
            mark=Msynth.paramindex[0]-1;
            while (mark>1 && Msynth.paramindex[mark]<0){
                printf("Rfreeindeks=%d,",mark);
                Msynth.paramindex[mark] *=-1; //aktiveerime taas õige indexi millele osutab viimane indeks
                Msynth.paramindex[0]--;
                mark--;
            }
            return 0;
        }
    } else Msynth.param[usv]=iv;
    Msynth.IP[prg]=*(unsigned short*)(Msynth.param+usv+2);//repeat back
    return 0;
case ')': //chords end, memory free
    mark=*(PC+Msynth.IP[prg])-48;
    while (mark<9){
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    //free mem
    Msynth.chords[prg]=0;
    Msynth.Saw[prg]=0; //Pole oluline
    Msynth.dU[prg]=0; //Pole oluline
    Msynth.paramindex[Msynth.chords[prg]] *=-1;//set free
    //free memory if possible
    mark=Msynth.paramindex[0]-1;
    while (mark>1 && Msynth.paramindex[mark]<0){
        Msynth.paramindex[mark] *=-1;
        Msynth.paramindex[0]--;
        mark--;
    }
    printf(" Chords END, last dynmem index=%d ",mark);
    return 0;
case '(': //chords begin, memory reserved
    Msynth.choind[prg]=Msynth.paramindex[0];//uus ineks
    if (Msynth.choind[prg]==MAXPARAMINDEXS) return -2;//no free indeks
    iv=Msynth.paramindex[Msynth.choind[prg]];//uus mälunumber
    Msynth.paramindex[0]++;
    printf("Chordstart Ind=%d,MOff=%d,",Msynth.choind[prg],iv);
    //printf("Ch freq=%f\n",Msynth.dUfreq[prg]);
    //unsigned char ch=0;
    Msynth.chords[prg]=0;
    //viitab
    mark=*(PC+Msynth.IP[prg]);
    while (mark!=')'){//Notes read
        switch (mark) {
            // 1,0594630943592952645618252949463  100 cents
            //1,0293022366434920287823718007739  50 cents
        case 'a': fv=27.5;
            printf(" a ");
            break;
        case 'b': fv=30.8677; //B & C vahel on poole peenem 50 cents
            break;
        case 'c': fv=16.3516; //C0 on kõige madalam noot üldse!!
            break;
        case 'd': fv=18.354;
            break;
        case 'e': fv=20.6017; //E ja F vahel on poole peenem
            break;
        case 'f': fv=21.8268;
            break;
        case 'g': fv=24.4997;
            break;
        case '+':
            if (fv==30.8677 || fv==20.6017) fv *=1.0293022366434920287823718007739; else fv *=1.0594630943592952645618252949463;
            break;
        case '-':
            if (fv==16.3516 || fv==21.8268) fv /=1.0293022366434920287823718007739; else fv /=1.0594630943592952645618252949463;
            break;
        //meibi octav
        case 'o':
            mark=*(PC+Msynth.IP[prg]);
            Msynth.IP[prg]++;
            fv=0; //okt ei sätest
            if (mark=='-') {// -
                if (Msynth.Octav[prg]) Msynth.Octav[prg]--;
                break;
            }
            if (mark=='+') {//+
                Msynth.Octav[prg]++;
                if (Msynth.Octav[prg]>11) Msynth.Octav[prg]=11;
                break;
            }
            //okt
            mark -=48;
            while (mark<9){
                fv=10*fv+mark;
                Msynth.IP[prg]++;
                mark=*(PC+Msynth.IP[prg])-48;
            }
            if (fv>11) fv=11;
            Msynth.Octav[prg]=fv;
            fv=0;
            break;
        } //end switch
        //chords note write oktav ei läheks arvesse
        if (fv>0){//dU set (Notes to dyn memory write
            fv *=1<<Msynth.Octav[prg];
            *(float*)(Msynth.param+iv)=fv*0xFFFF*Msynth.ScardT;
            printf("COff=%d,CdU=%f,",iv,*(float*)(Msynth.param+iv));
            Msynth.chords[prg]++;
            iv +=sizeof(float);//+4
        }
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg]);
    }//end chords while
    //if (Msynth.chords[prg]>8) return -7;//chords m>x limit
    //mark on vaba

    //printf("MOaftdU=%d,chords=%d,",iv,Msynth.chords[prg]);
    usv=Msynth.IP[prg];
    Msynth.IP[prg]++;//et oleks sulust järgmisele viit
    //Memory set

    //saw reserved, chords memory pointers know (ch)
    //Track based memory - memory offset hold in Saw igale trackile on oma !2B Saw x ch ! 3B bezier x ch
    mark=Msynth.firsGtrack[prg];
    printf("firstG=%x,",Msynth.Gprg[mark]&7);
    while (prg==Msynth.Gprg[mark]&7) {
        printf("first tr G=%d,const",mark);
        if (!(Msynth.wavef[prg]&16)){//only not const track
            Msynth.Saw[mark]=iv;
            printf("memi=%d,",iv);
            //nullida???
            iv +=sizeof(unsigned short)*Msynth.chords[prg];
            //bezier... indeks jääb alles, kuid bezier kolib siia ?selle nullimine?
            if (Msynth.Gprg[mark]&0xF8) iv +=3*Msynth.chords[prg];
        }
        mark++;
    }
    printf("Ch freq=%f\n iv=%d",Msynth.dUfreq[prg],iv);
    if (iv>MAXPARAMS) return -5;

    //End memory reserving
    mark=Msynth.paramindex[0];
    Msynth.paramindex[mark]=iv;
    printf("Chords=%d,Memoffset=%d,",Msynth.chords[prg],iv);
    printf("Ch freq=%f\n",Msynth.dUfreq[prg]);

    //sulgude järel olev taktimõõt
    iv=0;
    mark=*(PC+Msynth.IP[prg])-48;
    while (mark<9){
        iv=10*iv+mark;
        Msynth.IP[prg]++;
        mark=*(PC+Msynth.IP[prg])-48;
    }
    if (iv==0 || iv>32) iv=4;
    //ticks -time settings
    Msynth.n[prg][2]=120*Msynth.Scardf/(iv*Msynth.bpm[prg]);//täis
    //Sustain time calculated in Envelopes!
    printf("Chord Tickse=%d, dUpointer=%f,",Msynth.n[prg][2],Msynth.dU[prg]);
    Msynth.Estage[prg]=5;
    Msynth.IP[prg]=usv;
    return 0;
}
return 0;
}

static void SDLCALL Synth(void *userdata, SDL_AudioStream *astream, int addbytes, int totalbytes){
int tsyklen=addbytes/sizeof(short);
short buf[tsyklen];
float dLFO=14*M_PI*Msynth.ScardT;
unsigned short triangle,*sawp,*noisep;
unsigned char ch,chm,tr,cbit,cua,prgnr,testbit;
unsigned char ls;
char *paramp;
unsigned int trbit;
float fajut,*dUpointer;
float PI2dT=2*M_PI/(float)Msynth.Scardf;//vajalik konstant filters blokis
int h,volout;
//bezier, polynom, furie, PCM
unsigned char x,t0,t1,t2,*bezierp;
unsigned short b0,b1,b2;
float t;
float *poly,F;

for (h=0;h<tsyklen;h++){
    //program
    Msynth.LFO +=14*M_PI*Msynth.ScardT;
    if (Msynth.LFO>7) Msynth.LFO -=2*M_PI;
    for (tr=0;tr<Msynth.Programs;tr++){//Program part 1 **************************************************
        cbit=1<<tr;

        //Program instructions *********************************************************************
        //printf("F=%d,",)
        //if (Msynth.Test&cbit)
        //idee on - softsync ooteks lükatakse test välja
        if (Msynth.SSync&cbit) {
            Msynth.SSync &=!cbit;
            Msynth.Test |=cbit;
        }
        //Next instruction only if old note ended.
        if (Msynth.Test&cbit && Msynth.Estage[tr]==0) Sprocessor(tr);

        //Envelope ****************************
        if (Msynth.Estage[tr] && Msynth.Test&cbit){//Run only if stage >0 & test allowed
            //printf("Gen st=%d,prg=%d, ",Msynth.Estage[tr],tr);
            if (Msynth.Estage[tr]==6) {//Init Silence
                Msynth.Estage[tr]=1;
                Msynth.Ereg[tr]=0;
                Msynth.nxv[tr][1]=Msynth.n[tr][2];
            }
            if (Msynth.Estage[tr]==5) {//Init A
                //crescendo!!
                if (Msynth.crescendo[tr]){
                    volout=Msynth.MixVol[tr]+Msynth.crescendo[tr];
                    if (volout<0) volout=0;
                    if (volout>255) volout=255;
                    Msynth.MixVol[tr]=volout;
                }

                //find a stage for adsr -saaks lihtsamini julmalt lükata def
                //Siin arvutame a alusel kõik nx ajad - mis lähevvd nulli
                volout=Msynth.n[tr][2]-Msynth.n[tr][4]-Msynth.n[tr][3]-Msynth.n[tr][1];
                if (volout<0)  {
                    //aja ümberarvutamine ja only attack&decay used
                    //printf("Volneg");
                    Msynth.nxv[tr][4]=Msynth.n[tr][2]/(1+sqrt(-Msynth.a[tr][4]/Msynth.a[tr][3]));
                    Msynth.nxv[tr][3]=Msynth.n[tr][2]/(1+sqrt(Msynth.a[tr][3]/-Msynth.a[tr][4]));
                    //printf("lyhver Ka=%d,KD=%d\n",Msynth.nxv[tr][4],Msynth.nxv[tr][3]);
                    //Siin on viga!!! tickse tuleb liiga palju!!!

                    Msynth.nxv[tr][2]=0;
                    Msynth.nxv[tr][1]=0;
                    Msynth.nxv[tr][0]=0;
                } else { //
                    Msynth.nxv[tr][4]=Msynth.n[tr][4];
                    Msynth.nxv[tr][3]=Msynth.n[tr][3];
                    Msynth.nxv[tr][2]=volout;
                    Msynth.nxv[tr][1]=Msynth.n[tr][1];
                    Msynth.nxv[tr][0]=Msynth.n[tr][0];
                    //printf("Pikver S=%d\n",volout);
                }

                //Filter variant
                volout=Msynth.n[tr][2]-Msynth.n[tr][8]-Msynth.n[tr][7]-Msynth.n[tr][6];//Nt-aDR
                Msynth.nxf[tr][0]=0;//not important, may delete
                if (volout<0)  {//vaid a & D ja lühendvtud
                    //aja ümberarvutamine ja only attack&decay used
                    //note time n[2]
                    Msynth.nxf[tr][4]=Msynth.n[tr][2]/(1+sqrt(Msynth.af[tr][4]/-Msynth.af[tr][3]));
                    Msynth.nxf[tr][3]=Msynth.n[tr][2]/(1+sqrt(-Msynth.af[tr][3]/Msynth.af[tr][4]));
                    Msynth.nxf[tr][2]=0;
                    Msynth.nxf[tr][1]=0;
                } else {//ei ole vaja ümber arvutada
                    Msynth.nxf[tr][4]=Msynth.n[tr][8];
                    Msynth.nxf[tr][3]=Msynth.n[tr][7];
                    Msynth.nxf[tr][2]=volout;
                    Msynth.nxf[tr][1]=Msynth.n[tr][6];
                    //suslevele holds n[5], but it not needed
                }
                Msynth.Estage[tr]=4;//Volrun
                Msynth.Estagef[tr]=4;//filterrun
                //printf("st4 a=%d,",Msynth.nxv[tr][4]);
                //printf("D=%d,",Msynth.nxv[tr][3]);
                //printf("S=%d,",Msynth.nxv[tr][2]);
                //printf("R=%d,",Msynth.nxv[tr][1]);
                //printf("Ereg=%f\n",Msynth.Ereg[tr]);
            }
            //ADSR next value find
            //printf("!st=%d !",Msynth.Estage[tr]);
            //printf("nxv=%d,    st=%d !",Msynth.nxv[tr][Msynth.Estage[tr]],Msynth.Estage[tr]);
            if (Msynth.nxv[tr][Msynth.Estage[tr]]) {//Vol version
                //muutus
                Msynth.Ereg[tr] +=Msynth.a[tr][Msynth.Estage[tr]]*Msynth.nxv[tr][Msynth.Estage[tr]]*2;
                Msynth.nxv[tr][Msynth.Estage[tr]]--;
            } else { //Next settings -if stage=0, then stop automatly
                Msynth.Estage[tr]--;
                //printf("St=%d,Ereg=%f,tickse=%d!",Msynth.Estage[tr],Msynth.Ereg[tr],Msynth.nxv[tr][Msynth.Estage[tr]]);
            }
            //overvalue only
            if (Msynth.Ereg[tr]>0xFFFF) Msynth.Ereg[tr]=0xFFFF;
            //printf("midle   st=%d !",Msynth.Estage[tr]);
            //minus protection - end calculation
            if (Msynth.Ereg[tr]<=0) {
                Msynth.Ereg[tr]=0;
                //Mis iganes aga aeg tiksub lõpuni! (filter mitte!!!
            }
            //printf("End   st=%d !",Msynth.Estage[tr]);
        }
        //Filter envelope
        //ADSR next value find
        ch=Msynth.Estagef[tr];
        if (ch) {//Filter version
            Msynth.Eregf[tr] +=Msynth.af[tr][ch]*Msynth.nxf[tr][ch]*2;
            Msynth.nxf[tr][ch]--;
            //overvalue only
            if (Msynth.Eregf[tr]>0xFFFF) Msynth.Eregf[tr]=0xFFFF;

            //minus protection - end calculation
            if (Msynth.Eregf[tr]<=0) {
                Msynth.Eregf[tr]=0;
                Msynth.Estagef[tr]=0;//Filter stage always go OFF
            }
        } else { //Next settings
                Msynth.Estagef[tr]--;
        }


    }//PRG osa END  ******
    //printf("ENDPRG =%d",Msynth.Estage[0]);


    buf[h]=0;
    prgnr=MAXPROGRAMS; //for stvrt track we need false value
    trbit=1;
    for (tr=0;tr<Msynth.Generators;tr++){//Generator *************************************************************************************
        if (prgnr!=Msynth.Gprg[tr]) Msynth.MixVolOut[Msynth.Gprg[tr]]=0; //set Volout=0 if first new track
        prgnr=Msynth.Gprg[tr]&7; // |5b bezier param index|3b prg|
        cbit=1<<prgnr;

        if (Msynth.Test&cbit && Msynth.Estage[prgnr]) {//track work only if testbit is 1 & stage  0
            //Synth track, pointers
            sawp=&(Msynth.Saw[tr]);//def no chord version
            ch=(Msynth.Gprg[tr])>>3;
            bezierp=0; //see on null kui pole
            if (ch) bezierp=Msynth.param+Msynth.paramindex[ch];//bezier exist
            if (Msynth.wavef[tr]&16) {//const, no chord bezierp my exist
                dUpointer=&onef; //dU=1,

            } else { //chord?
                chm=Msynth.chords[prgnr];
                dUpointer=&(Msynth.dU[prgnr]); //no chord ver
                if (chm) {
                    //chord version
                    dUpointer=(float*)(Msynth.param+Msynth.paramindex[Msynth.choind[prgnr]]);//offset on dU (flp
                    //if (h==0) printf("C Off=%d, dUpointer=%f",Msynth.paramindex[Msynth.choind[prgnr]],*dUpointer);
                    //saw
                    triangle=Msynth.Saw[tr];
                    //if (h==0) printf("ch s w  off=%d,",triangle);
                    sawp=(unsigned short*)(Msynth.param+triangle);
                    //bezier asub nüüd peale saw
                    triangle +=chm*sizeof(unsigned short);
                    bezierp=Msynth.param+triangle;//pole oluline kontrollida, kui ei kasutata siis valeviit ei ohusta
                    chm--;  //chords for töötab muidu valesti
                }
            }

            Msynth.GenV[tr]=0;
            Msynth.HSync &=!cbit; //SYNC=0, only Sync track & 1 tact may be 1
            //wavef  ||x|aDSR|Sync|Const|4b wave|

            for(ch=0;ch<=chm;ch++){//chords
                fajut=*dUpointer;//for chords is offset cua. No chords then 0+prgnr+0=prgnr
                fajut *=Msynth.dUfreq[tr]; //duFreg on gene põhine



                if (cbit&Msynth.vibrato) fajut +=458745*Msynth.ScardT*sin(Msynth.LFO);//0xFFFF*7/48000=9
                fajut +=*Msynth.dUp[prgnr]*Msynth.dUpfreq[prgnr];//Modulation part is program põhine


                if (Msynth.wavef[tr]&16) fajut=1;//CONST no dU, used const
                triangle=*sawp;
                *sawp +=(unsigned short) fajut;//S +dUplus
                //Reset if set? Reset only works if track sync mask allowed && program mask is setted other program sync
                if (Msynth.HSync&Msynth.HSyncmaskprg[prgnr] && Msynth.HSyncmasktr&trbit) *sawp=0; //Sync reset (for all chords ) lubtud vaid kui syncmask lubab
                //Synctrack?
                if (triangle>*sawp) {//oli nullist läbiminek
                    ls=1;//local sync signal (only bezier use it
                    //SYNC  synctrack chords 0 & only sync track
                    if (ch==0 && Msynth.wavef[tr]&32) Msynth.HSync |=cbit;
                }

                triangle=*sawp; //et ei peaks koguaeg indeksit

                switch(Msynth.wavef[tr]&15){//waveform select
                case 0: //saw
                    Msynth.GenV[tr] +=(unsigned short)(triangle+0x7FFF);
                    //if (h==0 && ch==0) printf("du=%f",*dUpointer);
                    //printf("=%f,",Msynth.GenV[tr]);
                    break;
                case 1://triangle
                    if (triangle&0x8000) triangle=!triangle;
                    Msynth.GenV[tr] +=(unsigned short)   ((triangle<<1)+0x7FFF);
                    break;
                case 2://pulse
                    if (triangle>Msynth.PulseLevel[prgnr]) Msynth.GenV[tr] +=0xFFFF;
                    break;
                case 3://sinus
                    fajut=2*triangle*M_PI*.0000152587890625; // 1:0xFFFF
                    Msynth.GenV[tr] +=0x7FFF+0x7FFF*sin(fajut);//nihe põhjus on sin on +-väärtusega
                    break;
                case 4://noise
                    if (ls) *(unsigned short*)(bezierp+1)=0xFFFF&rand();
                    Msynth.GenV[tr] +=*(unsigned short*)(bezierp+1);
                    break;
                case 5: //Ringmodulation or LFO out
                    Msynth.GenV[tr] +=*(Msynth.Ringp[prgnr])*triangle*.0000152587890625; // 1:0xFFFF - vaja normaliseerida
                    break;
                case 6://Pulse modulation
                    if (triangle>*(Msynth.Ringp[prgnr])) Msynth.GenV[tr] +=0xFFFF;
                    break;
                case 7://min kahest signaalist
                    if (Msynth.GenV[tr]>*(Msynth.Ringp[prgnr])) Msynth.GenV[tr]=*(Msynth.Ringp[prgnr]);
                    break;
                case 8://PCM - pakkimata ezier 256 t väärtust, igaüks char
                    x=triangle>>8;
                    t=(float)(triangle&0xFF)/256;
                    if (ls) *bezierp=0;
                    Msynth.GenV[tr] +=(*(Msynth.PCM[prgnr]+*bezierp+1)*t-*(Msynth.PCM[prgnr]+*bezierp)*(1-t))*257;//lineaarne bezier 8bit, teisendatakse 16 bit
                    break;
                case 9://Bezier
                    //unsigned char *bi; //saw on t telje väärtus mis on max 256
                    //bi=(unsigned char*) &Msynth.spare[tr];
                    x=triangle>>8;//jooksev ajatelg
                    //unsigned char *bpointer;
                    //localsync!!!
                    if (ls) *bezierp=0;

                    //kontrolliv ja paikaja
                    //*bezierp on jooksev punkti indeks, mis peab olema t0 ja t1 vahel
                    triangle=*bezierp;
                    while (x<*(Msynth.PCM[prgnr]+*bezierp)) *bezierp +=3;//niikaua kuni x>t0
                    while (x>=*(Msynth.PCM[prgnr]+*bezierp+3) && *bezierp>3) *bezierp -=3; //niikaua kuni x<=t1
                    //ajatelje bezier koord
                    //mitu varianti, kasutame kõik uuesti, kuna indeksid võisid muutuda
                    //kui ei muutunud siis poleks vaja!
                    if (triangle!=*bezierp) {
                        t0=*((unsigned char*)Msynth.PCM[prgnr]+*bezierp);//unsigned char? 0..255
                        t1=*((unsigned char*)Msynth.PCM[prgnr]+*bezierp+3);
                        t2=0;
                        b0=*((unsigned short*) (Msynth.PCM[prgnr]+*bezierp+1));//y telje väärtused
                        b1=*((unsigned short*) (Msynth.PCM[prgnr]+*bezierp+4));
                        b2=0;//kui on tavaline
                        if (b1&0x8000) {//Bezier in B1
                            b2=b1<<1;
                            t2=t1;
                            t1=*((unsigned char*)Msynth.PCM[prgnr]+*bezierp+6);
                            b1=*((unsigned short*) (Msynth.PCM[prgnr]+*bezierp+7));
                        }
                        if (b0&0x8000) {//Bezier in B0
                            b2=b0<<1;
                            t2=t0;
                            if (*bezierp>2) {
                                t0=*((unsigned char*)(Msynth.PCM[prgnr]+*bezierp-3));
                                b0=*((unsigned short*) (Msynth.PCM[prgnr]+*bezierp-2));
                            } else return; //esimene punkt ei tohi oll bezier!!!
                        }
                        b0 <<=1;
                        b1 <<=1;
                    }

                    //find t
                    t=(t1-t0);
                    if (t==0) return; //ei saa olla punktide vahel x
                    t=(x-t0)/t;
                    //find value
                    if (t2) {//bezier root exist
                        Msynth.GenV[tr] +=t*t*b1;
                        t=1-t;
                        Msynth.GenV[tr] +=2*t*b2+t*t*b0;
                    } else Msynth.GenV[tr] +=b0+t*(b1-b0);
                    break;
                case 10://polynom
                    x= *(unsigned char*)Msynth.PCM;//võibolla ka floatiks teha
                    poly=(float*)Msynth.PCM;
                    poly++;
                    //on
                    fajut=triangle-0x7FFF;
                    Msynth.GenV[tr]=*poly;
                    poly++;
                    for (h=1;h<x;h++){
                        Msynth.GenV[tr] +=*poly*fajut;
                        fajut *=fajut;
                        poly++;
                    }
                    break;
                }//wAveform select end
                ls=0;
            }//chords end
            Msynth.GenV[tr] /=ch+1;//Normalization

            //Adsr Volume correcture *****************************************************************************************************
            //only if generator not const we use adsr
            if (!(Msynth.wavef[tr]&16)) {//Not const, use ADSR, constant not used
                fajut=Msynth.Ereg[prgnr];
                if (Msynth.wavef[tr]&32) fajut=Msynth.Ereg[MAXPROGRAMS+prgnr]; //Use Ereg1
                 Msynth.GenV[tr]*=fajut*.0000152587890625;
             }//Kasutame if - lihtsam, saaks ka ühe avaldisega
            //Mixing ******************************************************************************************************************
            //võib ju olla ka väljundis puhas constant generaator, keeld saavutatakse MixVol=0
            //find sum volume with constant track settings & modulation (LFO)
            //Mixp on programmipõhine aga MixpV on raja põhine!!!!
            fajut=Msynth.MixVol[tr]*.01 +*(Msynth.Mixp[prgnr])*.000000152587890625*Msynth.MixpV[tr]; //Me moduleerime Vol väärtust
            //Vol=ConstTRVol+*(GenV) x ConstTRVolGen -  See on mõeldud LFO modulatsiooniks. Gene on kõigiltrckidel üks ja sama aga Vol väärtusega saab sedv nullida või juurde mixida
            fajut *=(Msynth.GenV[tr]-0x7FFF);
            //printf("G=%f,",Msynth.GenV[tr]);//siin fajut on VOL
            Msynth.MixVolOut[prgnr] +=fajut;//siin toimub kanalite püsivol ja modul.
            trbit <<=1;
            sawp++;
            dUpointer++;
            noisep++;
            bezierp++;
        }//Gen END!
        //Filters ******************************************************************

        for(tr=0;tr<Msynth.Programs;tr++){
            //Filters only progr
            volout=0;
            if (Msynth.firsGtrack[tr]&128) {
                //High Pass beeta= 1/(2*M_PI*dT*fc+1)
                fajut=PI2dT*(Msynth.CutOffHP[tr]+*(Msynth.CutOffHPmodp[tr])*Msynth.CutoffHPrate[tr]);
                fajut=1/fajut;//beeta
                //y[i] := α × (y[i−1] + x[i] − x[i−1])
                Msynth.FOutHP[tr]=fajut*(Msynth.FOutHP[tr]+Msynth.MixVolOut[tr]-Msynth.FInoldHP[tr]);
                Msynth.FInoldHP[tr]=Msynth.MixVolOut[tr];
                volout=(int)Msynth.FOutHP[tr];
            }
            if (Msynth.firsGtrack[tr]&64) {
                //High Pass beeta=2PI*dT*fc/(2PI*dT*fc+1)
                fajut=PI2dT*(Msynth.CutOffLP[tr]+*(Msynth.CutOffLPmodp[tr])*Msynth.CutoffLPrate[tr]);
                fajut=fajut/(fajut+1);//beeta
                // y[i] := y[i-1] + α * (x[i] - y[i-1])
                Msynth.FOutLP[tr]=Msynth.FOutLP[tr]+fajut*(Msynth.MixVolOut[tr]-Msynth.FOutLP[tr]);
                volout +=(int)Msynth.FOutLP[prgnr];
            }

            //Program Mixer
            //ei olnud filtreid!!!
            if (!(Msynth.firsGtrack[tr]&0xC0)) volout=Msynth.MixVolOut[tr]; //Filtreid ei olnud,
            //last program volume
            volout *=Msynth.MixPV[tr]*.01;
            volout +=buf[h];
            //piiritingimused
            if (volout>32767) volout=32767;//piiritingimused
            if (volout<-32768) volout=-32768;
            //printf("V=%d",volout);
            buf[h]=volout;
        }//end filter & Mixer

    }
}
SDL_PutAudioStreamData(astream, buf,addbytes);
return;
}


//******************** INIT********************************************

float Zerof=0;
char *Zeroc="\0";

int SynthInit(SynthInstr_t *SDB, char *program){
unsigned char prgs=Msynth.Programs;
if (prgs>=MAXPROGRAMS) return -2;
Msynth.Programs++;

unsigned char cg=Msynth.Generators;
if (cg>=MAXGENERATORS) return -cg;
unsigned char pi=1<<(Msynth.Programs-1);
unsigned char gi=0;
unsigned short flo=SDB->flags[0];
printf("Flags %X,",flo);
//Interrupts signals -
Msynth.Test |=pi; //Genrator vaatab kah oma test bitti tööks -selle genereerib aDSR
//printf("T=%x",Msynth.Test);
pi=0;
Msynth.SSync=0; //SoftSyncis ei ole maski - iga track vaatab oma bitti

Msynth.HSync=0;//genreerib ainult Sync bitti omav chords=0.

//Program***************************************************************
Msynth.bpm[prgs]=120;
Msynth.firsGtrack[prgs]=cg;//first track nr for program
Msynth.Octav[prgs]=SDB->octav;
printf("Oct=%d,",SDB->octav);

Msynth.PRG[prgs]=program; //char *PRG[MAXPROGRAMS];
Msynth.IP[prgs]=0;//unsigned short IP[MAXPROGRAMS];
Msynth.firsGtrack[prgs]=cg;
Msynth.repeats[prgs]=0;
Msynth.crescendo[prgs]=0;
//wave sets
Msynth.PulseLevel[prgs]=SDB->PulseLevel; //unsigned short PulseLevel[MAXPROGRAMS];
printf("Pulselevel=%x",Msynth.PulseLevel[prgs]);

//Master Volume (unsigned char type)
Msynth.MixPV[prgs]=100; //see on kasutaja vol määrata, algsätted seda ei näpi - vaikesäte on 100

//all modulation pointers zero
//  |2b FHP|2b FLP|*LPmod!*HPmod|*Mixp!*Ringp|     |*dUp|E|S|C|4b wave|
Msynth.dUp[prgs]=&Zerof;
Msynth.dUpfreq[prgs]=0; //Modulation zero float dUpfreq[MAXPROGRAMS];//Used freq & Pulse modulation

Msynth.Ringp[prgs]=&Zerof;
Msynth.Mixp[prgs]=&Zerof;
//Program generator sets
Msynth.dU[prgs]=0; //float dU[MAXPROGRAMS*MAXCHORDS]; pole oluline -rohkem, et kogemata ei käivituks

//Filters************************************************************************
// !2b FHP!2b FLP!*LPmod!*HPmod!*Mixp!*Ringp!     !*dUp!E!S!C!4b wave!
pi=flo>>14;

switch (pi) {
case 3:    //yes but set is in gener
case 0: //No modul
    Msynth.CutOffHPmodp[prgs]=&Zerof;
    printf("NoHPfilter");
    break;
case 1://Ereg 0
    Msynth.CutOffHPmodp[prgs]=&(Msynth.Ereg[prgs]);
    break;
case 2:    //Ereg 1
    Msynth.CutOffHPmodp[prgs]=&(Msynth.Ereg[MAXPROGRAMS+prgs]);
    break;
}
pi=(flo>>12)&3;
switch (pi) {
case 3:    //yes but set is in gener
case 0: //No modul
    Msynth.CutOffLPmodp[prgs]=&Zerof;
    printf(" NoLPfilter");
    break;
case 1://Ereg 0
    Msynth.CutOffLPmodp[prgs]=&(Msynth.Ereg[prgs]);
    break;
case 2:    //Ereg 1
    Msynth.CutOffLPmodp[prgs]=&(Msynth.Ereg[MAXPROGRAMS+prgs]);
    break;
}
Msynth.CutOffHP[prgs]=SDB->CutOfffreqHP;//float CutOffHP[MAXPROGRAMS];
Msynth.CutOffLP[prgs]=SDB->CutOfffreqLP;//float CutOffLP[MAXPROGRAMS];
Msynth.CutoffHPrate[prgs]=SDB->CutOfffreqHPmodR;
Msynth.CutoffLPrate[prgs]=SDB->CutOfffreqLPmodR;
pi=flo>>8;
Msynth.firsGtrack[prgs] |=pi&0xC0; //Filtri flags bitid
pi=0;

//start zero Filters
Msynth.FOutHP[prgs]=0;
Msynth.FOutLP[prgs]=0;
Msynth.FInoldHP[prgs]=0;
Msynth.MixVolOut[prgs]=0;//igaks juhuks nulli
Msynth.Estage[prgs]=0;
Msynth.Estagef[prgs]=0;
Msynth.Ereg[prgs]=0;
Msynth.Eregf[prgs]=0;

//Envelopes****************************************************************
//konvert ms to Soundcard ticks
Msynth.n[prgs][4]=SDB->adsr[0]*Msynth.Scardf*.001;
Msynth.n[prgs][3]=SDB->adsr[1]*Msynth.Scardf*.001;
Msynth.n[prgs][2]=0;//Note Time
Msynth.n[prgs][1]=SDB->adsr[3]*Msynth.Scardf*.001;
Msynth.n[prgs][0]=SDB->adsr[2];//Suslevel
//Filter envelope
Msynth.n[prgs][8]=SDB->adsr[4]*Msynth.Scardf*.001;
Msynth.n[prgs][7]=SDB->adsr[5]*Msynth.Scardf*.001;
Msynth.n[prgs][6]=SDB->adsr[7]*Msynth.Scardf*.001;
Msynth.n[prgs][5]=SDB->adsr[6];//suslevel
//a constant to attack t=0 then 0xFFFF,
if (Msynth.n[prgs][4]) Msynth.a[prgs][4]=0xFFFF/(float)(Msynth.n[prgs][4]*Msynth.n[prgs][4]); else {
    Msynth.n[prgs][4]=1;
    Msynth.a[prgs][4]=0xFFFF;
}
//Decay, minus
if (Msynth.n[prgs][3]) Msynth.a[prgs][3]=(SDB->adsr[2]-0xFFFF)/(float)(Msynth.n[prgs][3]*Msynth.n[prgs][3]); else {
    Msynth.n[prgs][3]=1;
    Msynth.a[prgs][3]=SDB->adsr[2]-0xFFFF;
}
Msynth.a[prgs][2]=0;
//Release, minus. kui suslevel=0 siis R=0
//kui t=0 siis a=suslevel, t=1
if (Msynth.n[prgs][1]) Msynth.a[prgs][1]=SDB->adsr[2]/(float)(Msynth.n[prgs][1]*Msynth.n[prgs][1]); else {
    Msynth.n[prgs][1]=1;
    Msynth.a[prgs][1]=-SDB->adsr[2];
}
//Filter var.
if (Msynth.n[prgs][8]) Msynth.af[prgs][4]=0xFFFF/(float)(Msynth.n[prgs][8]*Msynth.n[prgs][8]); else {
    Msynth.n[prgs][8]=1;
    Msynth.af[prgs][4]=0xFFFF;
}
//Decay, minus
if (Msynth.n[prgs][7]) Msynth.af[prgs][3]=(SDB->adsr[7]-0xFFFF)/(float)(Msynth.n[prgs][7]*Msynth.n[prgs][7]); else {
    Msynth.n[prgs][7]=1;
    Msynth.af[prgs][3]=SDB->adsr[6]-0xFFFF; //=Suslevel-0xFFFF
}
Msynth.af[prgs][2]=0;//suslevel a typic 0

//Release, minus. kui suslevel=0 siis R=0
//kui t=0 siis a=suslevel, t=1
if (Msynth.n[prgs][6]) Msynth.af[prgs][1]=SDB->adsr[6]/(float)(Msynth.n[prgs][6]*Msynth.n[prgs][6]); else {
    Msynth.n[prgs][6]=1;
    Msynth.af[prgs][1]=-SDB->adsr[6];
}



Msynth.dUpfreq[prgs]=SDB->dUpfreq;
//track inits
while ((SDB->freq[gi])!=0 && gi<6){
    if (Msynth.Generators>=MAXGENERATORS) return -MAXGENERATORS;
    Msynth.Generators++;
    flo=SDB->flags[gi];
    Msynth.dUfreq[cg]=SDB->freq[gi];//see võib olla
    Msynth.MixVol[cg]=SDB->Vol[gi]; //const may sometimes also go output
    Msynth.wavef[cg]=flo&127; //generator wave+ const.+sync+envelope
    printf("Msynth.wavef nr=%d,F=%X  !,",cg,Msynth.wavef[cg]&15);
    //      FL!FH!xxx  !LPmod!HPmod!*Mixp!*Ringp!*dUp!E!S!C!4b wave!

    //pointerid mis on jooksva generaatori aadressid
    if (flo&128) {
        Msynth.dUp[prgs]=&(Msynth.GenV[cg]);
        Msynth.dUpfreq[prgs]=*(float*)(SDB->params+pi);
        pi +=4; //float type len
    }
    if (flo&256) Msynth.Ringp[prgs]=&(Msynth.GenV[cg]);
    if (flo&512) Msynth.Mixp[prgs]=&(Msynth.GenV[cg]);
    if (flo&1024)  Msynth.CutOffHPmodp[prgs]=&(Msynth.GenV[cg]);
    if (flo&2048) Msynth.CutOffLPmodp[prgs]=&(Msynth.GenV[cg]);
    //LFO mixvol settings
    Msynth.MixpV[cg]=0;//defMsynth.MixpV[cg]ult it is 0
    if (flo&4096) {
        Msynth.MixpV[cg]=*(unsigned char*)(SDB->params+pi); //MixVol modulation pointer
        pi++;
    }
    //gen.põhised vahemuutujad nulli
    Msynth.Saw[cg]=0;
    Msynth.GenV[cg]=0;
    Msynth.chords[cg]=0;
    gi++;
    cg++;
    if (cg>=MAXGENERATORS) return -cg;
}
Msynth.Gprg[gi]=prgs+1;
return 0;
}


unsigned char harm[12]={0,63,10,70,25,40,40,0,200,63,255,65};
unsigned char bezi=0;
unsigned char bezt=0;

char *user={"blab"};
static void SDLCALL FeedTheAudioStreamMore(void *userdata, SDL_AudioStream *astream, int additional_amount, int total_amount)
{
    /* total_amount is how much data the audio stream is eating right now, additional_amount is how much more it needs
       than what it currently has queued (which might be zero!). You can supply any amount of data here; it will take what
       it needs and use the extra later. If you don't give it enough, it will take everything and then feed silence to the
       hardware for the rest. Ideally, though, we always give it what it needs and no extra, so we aren't buffering more
       than necessary. */
    //printf("add=%d,tot=%d",additional_amount,total_amount);
additional_amount /= sizeof (short);  /* convert from bytes to samples */
short buf[additional_amount];
char *ud;
ud=(char*)userdata;
printf("%s\n",ud);
int h;
//additional -mitu B tuleb kindlasti saata, total - mõnikord võib ka rohkem!
buf[0]=0;
float u,v,t1m,f,t;
if (bezi==0) printf("first!!!\n");
unsigned char ch;
unsigned char t0=harm[bezi];
unsigned char t1=harm[bezi+2];
unsigned char t2=harm[bezi+4];
unsigned char b0=harm[bezi+1];
unsigned char b1=harm[+bezi+3];
unsigned char b2=harm[+bezi+5];
//printf("\nS i=%d,t=%d,!%d,%d,%d,%d,",bezi,bezt,t0,b0,t1,b1);

for (h=0;h<additional_amount;h++){
    ch=bezi;
    //printf("E I=%d,B=%d,",bezi,bezt);
    while ((bezt<harm[bezi] || bezt>=harm[bezi+2]) && harm[bezi+2]<255) bezi+=2;
    //if (harm[bezi]==255) {printf("Lohki! t=%d",bezt);bezi=0;bezt=0;}
    //printf("P I=%d,Bt=%d,t0=%d\n",bezi,bezt,harm[bezi]);
    //while (( bezt>=harm[bezi+2] || bezt<harm[bezi]) && bezi>1) bezi-=2;//vähetõenäoline
    //while üle positsioneerib t0 j t1 v
    if (bezt==255) {//Endpoint
        buf[h]=harm[bezi+2] *255-0x7FFF;
        printf("End=%d\n",buf[h]);
        bezt=0;
        bezi=0;
    } else {
        if (ch!=bezi || h==0) {
            printf("Ch=%d,%d!\n",harm[bezi],harm[bezi+2]);
            t0=harm[bezi];
            t1=harm[bezi+2];
            b0=harm[bezi+1];
            b1=harm[bezi+3];
            b2=0;
            t2=0;
            if (b0&128) {
                b2=b0<<1;
                t2=t0;
                printf("Vnihe\n");
                if (bezi<2) return;//esimene punkt ei tohi olla bezpunkt
                t0=harm[bezi-2];
                b0=harm[bezi-1];
            }
            if (b1&128) {
                b2=b1<<1;
                t2=t1;
                printf("Pnihe\n");
                if (harm[bezi]==255) return; //viimane punkt ei tohi olla bezpunkt
                t1=harm[bezi+2];
                b1=harm[bezi+3];
            }
            b0 <<=1;
            b1 <<=1;
        }

        //bezier joonistamine
        //t arvutus on sama line kui ruut
        //printf("bezt=%d,t0=%d,t1=%d,t2=%d",bezt,t0,t1,t2);

        u=(float) (bezt-t0);
        v=(float) (t1-t0);
        //printf("u=%f,v=%f!",u,v);
        //t=0;


        if ((u-v)==0) return;
        t=(float)u/(float) v;
        t1m=1-t;

        //lõppväärtus
        if (t2) f=t1m*(t1m*b0+t*b2)+t*(t1m*b2+t*b1); else f=t1m*b0+t*b1;
        f *=255;
        buf[h]=(short)(f-0x7FFF);
        printf(" %d",buf[h]);
        //või joonistame?
        bezt++;

    }




}

printf("%d",bezt);
SDL_PutAudioStreamData(astream, buf,  additional_amount* sizeof (short));
return;
}


int main(){
printf("Hello world! %d,%d\n",bezi,bezt);
Msynth.Scardf=48000;
Msynth.ScardT=1/(float)48000;
Msynth.vibrato=0;
Msynth.tremolo=0;
Msynth.Programs=0;
Msynth.Generators=0;
//set dyn memory
Msynth.paramindex[0]=1;
Msynth.paramindex[1]=0;

char *program="o4Va1Vc11(bc)2<d1>";
SynthInit(&singlegen,program);
printf("tr=%d,%d",Msynth.Generators,Msynth.Programs);
//return 0;


    // returns zero on success else non-zero
    //
if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) {
    printf("error initializing SDL: %s\n", SDL_GetError());
}
SDL_Window* win = SDL_CreateWindow("GAME",800, 600, 0);//Loob akna kuskil süsteemimälus
//renderer=SDL_CreateRenderer(win,NULL);
SDL_Surface* gScreenSurface = SDL_GetWindowSurface(win);//saame ligipääsu süsteemi mälus loodud aknale
//SDL_LockSurface(gScreenSurface);
static SDL_AudioStream *stream = NULL;

SDL_AudioSpec spec;
spec.channels = 1;
spec.format =SDL_AUDIO_S16;
spec.freq = 48000;//SDL_AUDIO_F32

stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Synth, NULL);
if (!stream) {
    SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
    return -7;//SDL_APP_FAILURE;
}

    /* SDL_OpenAudioDeviceStream starts the device paused. You have to tell it to start! */
    SDL_ResumeAudioStreamDevice(stream);






//stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,NULL,NULL);
 //stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,FeedTheAudioStreamMore , user);

if (!stream) return -2;
SDL_ResumeAudioStreamDevice(stream);

printf("heli ");

//Kui me loome ise siis tekib tavamällu
//võime ohutuse mõttes lugeda
//printf("X=%d,Y=%d, pixinroe=%d, pixFormat=%x\n",gScreenSurface->h,gScreenSurface->w,gScreenSurface->pitch, gScreenSurface->format);
//SDL_PIXELFORMAT_XRGB8888
unsigned int *pixelp;
unsigned char pcm[256];

unsigned char bezier[8]={0,0,50,10,200,255,255,0};

pixelp=(unsigned int *)gScreenSurface->pixels;
int h=0,i=0,x=0,y;
//unsigned char harm[10]={100,67,55,19,5,2,1,1,0,0};
float m=0,n,dt,f,fs=200,g,t,b[3]={-.1,0,.1};
t=0;
//furie variant
for (i=1;i<10;i++) m +=harm[i];
m=.01/m;
//n=48000/fs;

//printf("dt=%f\n",dt);
unsigned char bi=0;
//for (h=0;h<8;h++) printf("B=%d,",bezier[h]);
float u,v,t1m;

unsigned char b0,b1,b2=0,t0,t1,t2=0;

t0=bezier[0];
b0=(bezier[1])<<1;
t1=bezier[2];
b1=(bezier[3])<<1;
bi=3;
if (bezier[3]&128) {
    //printf("BS");
    t2=t1;
    b2=b1;
    t1=bezier[4];
    b1=(bezier[5])<<1;
    bi=5;
}

for (x=0;x<256;x++) {
    //printf("x=%d,B=%d",x,bezier[bi+2]);

    if (x>=t1) {//jõudnud järgmisesse punkti
        //printf("N");
        t0=t1;
        b0=b1;
        bi++;
        if (bi>7) break;
        if (bezier[bi+1]&128) {
            //bezier kõverpunkt -tunnuseks t2!=0
            t2=bezier[bi];
            bi++;
            b2=(bezier[bi])<<1;
            //dprintf("BE=%d,=%d!",bezier[bi],b2);
            bi++;
        } else {t2=0; b2=0;}

        t1=bezier[bi];
        bi++;
        b1=(bezier[bi])<<1;
        //printf("HO=%d, B0=%d,B1=%d \n",bi,bezier[bi+1],bezier[bi+3]);
    }

    //t arvutus on sama
    u=(float) (x-t0);
    v=(float) (t1-t0);
    //printf("u=%f,v=%f!",u,v);
    //t=0;
    if ((u-v)==0) return -2;
    t=(float)u/(float) v;
    t1m=1-t;

    //lõppväärtus
    if (t2) f=t1m*(t1m*b0+t*b2)+t*(t1m*b2+t*b1); else f=t1m*b0+t*b1;

    //printf("f=%f,t=%f!",f,t);
    //printf("x=%d,bi=%d,B0=%d,B1=%d,f=%f!",x,bi,bezier[bi+2],bezier[bi+2],f);
    y=(int)(400-f);
    *(pixelp+x+y*800)=0x00FF00FF;

}





SDL_UpdateWindowSurface(win);
while (1) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
		// check event type
    if (event.type==SDL_EVENT_KEY_DOWN) {SDL_Quit(); return 0;}
    }
}
SDL_Quit();
return 0;



}
