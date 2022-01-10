// flanc montant
class PositivEdge {
  private :
    boolean memPrevState;
    boolean out;
  public :
    PositivEdge(boolean condition); //constructor
    boolean eval(boolean condition);
    boolean get_out();
};
PositivEdge::PositivEdge(boolean condition) {
  this->memPrevState = condition;
}
boolean PositivEdge::eval(boolean condition) { //update positiv edge state must be done ONLY ONCE by loop cycle
  out = condition && !memPrevState;
  memPrevState = condition;
  return out;
}
boolean PositivEdge::get_out() {  //use get_out() to know positiv edge state (use more than once by cycle is possible)
  return this < - out;
} // fin flanc montant

class OnDelayTimer {

  private :
    unsigned long presetTime = 1000;
    unsigned long memStartTimer = 0;            //memory top timer activation
    unsigned long elpasedTime = 0;             //elapsed time from start timer activation
    boolean memStartActivation;                //for positive edge detection of the activation condition
    boolean outTimer;                          //timer's out : like normally open timer switch
  public :
    OnDelayTimer(unsigned long _presetTime);   //constructor
    boolean updtTimer(boolean activation);      //return tempo done must be executed on each program scan
    unsigned long get_elapsedTime();           //return
    set_presetTime(unsigned long _presetTime); //change defaut preset assigned when instance created
    boolean get_outTimer();

};//end class OnDelayTimer
//constructor
OnDelayTimer::OnDelayTimer(unsigned long presetTime) {
  this -> presetTime = presetTime;
}
boolean OnDelayTimer::updtTimer(boolean activation) {
  if (!activation) {
    elpasedTime = 0;
    memStartActivation = false;
    outTimer = false;
    return false;
  } else {

    if (!memStartActivation) {
      memStartTimer = millis();
      memStartActivation = true;
    }
    elpasedTime = millis() - memStartTimer;
    outTimer = elpasedTime >= this->presetTime; //update timer 's "switch"
    return  outTimer;

  }
}//end endTimer()
//constructor
boolean OnDelayTimer::get_outTimer() {

  return this->outTimer;
}

// pin de sortie 22->27
const int iPIN_EV = 22; // EV
const int iPIN_T1 = 23; // T1
const int iPIN_T2 = 24; // T2
const int iPIN_T3 = 25; // T3
const int iPIN_H_AU = 26;

//pin entrée NO 40->46
const int iPIN_dcy = 40; // dcy
const int iPIN_raz = 41; // raz
// pin entrée NF 47->49
const int iPIN_acy = 47; // acy
const int iPIN_AU = 48;
//boolean entrée
boolean dcy, acy, raz, au = 0;
//boolean sortie
boolean T1, T2, T3, EV, H_AU = 0;
boolean almMaint = 0;

// nombre étapes et transistions
const unsigned int nbStep = 8;
const unsigned int nbTransition = 8;
boolean Step[nbStep], AU[2];
boolean transition[nbTransition], transtitionAU[2];
unsigned int cptCycles = 0;

//déclaration débug
String strDebugLine;
unsigned int stp, stpAU = 0 ; // étape numéro x dans le debug

// déclaration des flanc montant à getter: PositivEdge nomDeVariable(nomDeVariable à évaluer)
PositivEdge posEdge_dcy(dcy);
PositivEdge posEdge_raz(raz);
PositivEdge posEdge_cycles(Step[1]);
// déclaration timer : OnDelayTimer nomDeVariable(temps en milliseconde);
OnDelayTimer timerStep1(3000);
OnDelayTimer timerStep2(3000);
OnDelayTimer timerStep3(3000);
OnDelayTimer timerStep5(3000);
OnDelayTimer timerStep6(3000);
OnDelayTimer timerStep7(3000);

void setup() {
  Serial.begin(9600); // déclaration moniteur série
  // sortie
  pinMode(iPIN_T1, OUTPUT);
  pinMode(iPIN_T2, OUTPUT);
  pinMode(iPIN_T3, OUTPUT);
  pinMode(iPIN_EV, OUTPUT);
  pinMode(iPIN_H_AU, OUTPUT);
  //entrée
  pinMode(iPIN_dcy, INPUT);
  pinMode(iPIN_acy, INPUT);
  pinMode(iPIN_raz, INPUT);
  pinMode(iPIN_AU, INPUT);

  Step[0] = true; // début Step
  AU[0] = true;
}

void loop() {
  //lecture entrée
  dcy = digitalRead (iPIN_dcy);
  acy = digitalRead (iPIN_acy);
  raz = digitalRead (iPIN_raz);
  au = digitalRead (iPIN_AU);

  //evaluation flanc montant
  posEdge_dcy.eval(dcy);
  posEdge_raz.eval(raz);
  posEdge_cycles.eval(Step[7]);
  // déclaration des transitions
  transition[0] = Step[0] && posEdge_dcy.get_out() && acy && AU[1] && !almMaint;
  transition[1] = Step[1] && timerStep1.get_outTimer();
  transition[2] = Step[2] && timerStep2.get_outTimer();
  transition[3] = Step[3] && timerStep3.get_outTimer();
  transition[4] = Step[4] && !acy;
  transition[5] = Step[5] && timerStep5.get_outTimer();
  transition[6] = Step[6] && timerStep6.get_outTimer();
  transition[7] = Step[7] && timerStep7.get_outTimer();

  if (transition[0]) {
    Step[0] = false;
    Step[1] = true;
  }
  if (transition[1]) {
    Step[1] = false;
    Step[2] = true;
  }
  if (transition[2]) {
    Step[2] = false;
    Step[3] = true;
  }
  if (transition[3]) {
    Step[3] = false;
    Step[4] = true;
  }
  if (transition[4]) {
    Step[4] = false;
    Step[5] = true;
  }
  if (transition[5]) {
    Step[5] = false;
    Step[6] = true;
  }
  if (transition[6]) {
    Step[6] = false;
    Step[7] = true;
  }
  if (transition[7]) {
    Step[7] = false;
    Step[0] = true;
  }

  //AU
  transtitionAU[0] = AU[0] && !AU;
  transtitionAU[1] = AU[1] &&  AU;

  if (transtitionAU[0]) {
    AU[0] = false;
    AU[1] = true;
  }
  if (transtitionAU[1]) {
    AU[1] = false;
    AU[0] = true;
  }

  if (AU[1]) {
    Step[0] = true;
    for (int i = 1; i < nbStep; i++) {
      Step[i] = false;
    }
  }


  if (posEdge_cycles.get_out()) {
    cptCycles ++;
  }
  if (cptCycles >= 3) {
    almMaint = 1;
  } else if (posEdge_raz.get_out() && almMaint) {
    cptCycles = 0;
    almMaint = 0;
  }

  //sortie activée par Step (sortie = Step[x])
  EV = Step[4] && AU[0];
  T3 = !Step[0] && AU[0];
  T2 = (Step[2] || Step[3] || Step[4] || Step[5] || Step[6]) && AU[0];
  T1 = (Step[3] || Step[4] || Step[5]) && AU[0];
  H_AU = almMaint;

  // timer update (s'active à l'étape x)
  timerStep1.updtTimer(Step[1]);
  timerStep2.updtTimer(Step[2]);
  timerStep3.updtTimer(Step[3]);
  timerStep5.updtTimer(Step[5]);
  timerStep6.updtTimer(Step[6]);
  timerStep7.updtTimer(Step[7]);

  //association Sortie-Pin
  digitalWrite (iPIN_EV, EV);
  digitalWrite (iPIN_T1, T1);
  digitalWrite (iPIN_T2, T2);
  digitalWrite (iPIN_T3, T3);
  digitalWrite (iPIN_H_AU, H_AU );

  //debug: étapes active
  for (int i = 0; i < nbStep; i++) {
    if (Step[i]) {
      stp = i;
      break;
    }
  }
  for (int i = 0; i < 2 ; i++) {
    if (AU[i]) {
      stpAU = i;
      break;
    }
  }
  // sortie debug
  strDebugLine = "Step:" + String(stp, DEC) + " StepAU:" + String(stpAU, DEC) +
                 " dcy:" + String(dcy, DEC) + " acy:" + String(acy, DEC) + " raz:" + String(raz, DEC) +
                 " cycles:" + String(cptCycles, DEC) + " almMaint:" + String(almMaint, DEC) +
                 " EV:" + String(EV, DEC) + " T1:" + String(T1, DEC) + " T2:" + String(T2, DEC) + " T3:" + String(T3, DEC);
  Serial.println(strDebugLine);
}
