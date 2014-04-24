
#include <Trade\Trade.mqh>

#import "Strat.dll"
int get_algo(string base, string quote, string path);
int process_tick(int algo_p, string time, double close, double stop_loss);
int delete_algo(int algo_p);
#import

int algo_p = -1;

const int NO_PRE_PROCESS_TICKS = 25 + 270;
const int HOLD_W = 45;

int OnInit(void){
   
   algo_p = get_algo("EUR", "USD", "C:/workspace/Strat/back_test_files/EURUSD_min_2013.csv");
   
   if(algo_p == -1){
   
      Print("Failed to create algo object");
      return(INIT_FAILED);
   }
   
   MqlRates rt[295];
   if(CopyRates(_Symbol,_Period,0,NO_PRE_PROCESS_TICKS,rt)!= NO_PRE_PROCESS_TICKS){
   
      Print("CopyRates of ",_Symbol," failed, no history");
      return(INIT_FAILED);
   }
   
   for(int i = 0; i < NO_PRE_PROCESS_TICKS; i++){
   
      //dt_str format yyyy.mm.dd hh:mi
      //todo check dt_str convert correctly in algo obejct
      string dt_str =  TimeToString(rt[i].time, TIME_DATE|TIME_MINUTES);
      process_tick(algo_p, dt_str, rt[i].close, 0.01);
   }
     
   return(INIT_SUCCEEDED);
 }

void OnTick(void){
   //calculate stop
   //calculate size
     
   MqlTick last_tick;
   int signal = 0;
   if(SymbolInfoTick(_Symbol,last_tick)){
   
      string dt_str =  TimeToString(last_tick.time, TIME_DATE|TIME_MINUTES);
      signal = process_tick(algo_p, dt_str, last_tick.last, 0.01);
   }
   else{
   
      Print("OnTick:SymbolInfoTick() failed, error = ",GetLastError());
      return;
   }   
   
   if(signal != 0){
   
      double size = 0.01;
   
      CTrade trade;
      trade.PositionOpen(_Symbol, signal == 1 ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, size,
                      SymbolInfoDouble(_Symbol, signal == 1 ? SYMBOL_ASK : SYMBOL_BID),
                      0.01, 0);
   }
   
   datetime pos_open_dt = PositionGetInteger(POSITION_TIME);
   if(pos_open_dt != 0 && last_tick.time >= pos_open_dt + HOLD_W * 60){
   
      CTrade trade;
      trade.PositionClose(_Symbol,100);
      //todo check close result
   }
}

void OnDeinit(const int reason){

}
