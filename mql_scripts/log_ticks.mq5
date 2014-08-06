

#import "strat.dll"
void log_tick(string time, double ask, double bid, double last, ulong volume);
#import

void OnTick(void){
     
   MqlTick last_tick;
   
   if(SymbolInfoTick(_Symbol,last_tick)){
   
      string dt_str =  TimeToString(last_tick.time, TIME_DATE|TIME_MINUTES|TIME_SECONDS);
      log_tick(dt_str, last_tick.ask, last_tick.bid, last_tick.last, last_tick.volume);     
   }
   else{
   
      Print("OnTick:SymbolInfoTick() failed, error = ",GetLastError());
      return;
   }  
}
