
#include <Trade\Trade.mqh>

#import "Strat.dll"
ulong get_algo(string base, string quote, string path);
int process_tick(ulong algo_p, string time, double ask, double bid, double close, double stop_loss);
int delete_algo(ulong algo_p);
#import

ulong algo_p = -1;

const int NO_PRE_PROCESS_TICKS = 25 + 270;
const int HOLD_W = 45;

int OnInit(void){
   
   algo_p = get_algo("eur", "usd", "C:\\workspace\\Strat\\back_test_files\\Calendar-2013.csv");
   //algo_p = get_algo("eur", "usd", "C:\\workspace\\Strat\\back_test_files\\Calendar-04-27-2014.csv");
   
   if(algo_p == -1){
   
      Print("Failed to create algo object");
      return(INIT_FAILED);
   }
   
//   MqlRates rt[295];
//   if(CopyRates(_Symbol,_Period,0,NO_PRE_PROCESS_TICKS,rt)!= NO_PRE_PROCESS_TICKS){
//   
//      Print("CopyRates of ",_Symbol," failed, no history");
//      return(INIT_FAILED);
//   }
//   
//   for(int i = 0; i < NO_PRE_PROCESS_TICKS; i++){
//   
//      //dt_str format yyyy.mm.dd hh:mi
//      //todo check dt_str convert correctly in algo obejct
//      string dt_str =  TimeToString(rt[i].time, TIME_DATE|TIME_MINUTES);
//      process_tick(algo_p, dt_str, rt[i].close, rt[i].close, rt[i].close, 0.01);
//   }
     
   return(INIT_SUCCEEDED);
 }

void OnTick(void){
   //calculate stop
   //calculate size
     
   MqlTick last_tick;
   int signal = 0;
   if(SymbolInfoTick(_Symbol,last_tick)){
   
      string dt_str =  TimeToString(last_tick.time, TIME_DATE|TIME_MINUTES);
      signal = process_tick(algo_p, dt_str, last_tick.ask, last_tick.bid, last_tick.last, 0.01);     
   }
   else{
   
      Print("OnTick:SymbolInfoTick() failed, error = ",GetLastError());
      return;
   }   
   
   if(signal != 0 && !PositionSelect(_Symbol)){
      
      double size = 0.05;
   
      Print("Opening position at ", last_tick.time);
      CTrade trade;
      double price = SymbolInfoDouble(_Symbol, signal == 1 ? SYMBOL_ASK : SYMBOL_BID);
      trade.PositionOpen(_Symbol, signal == 1 ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, size,
                         price, price - signal * 0.01, price + signal * 0.05);
   }
   
   if(PositionSelect(_Symbol)){
   
      datetime pos_open_dt = PositionGetInteger(POSITION_TIME);
      
      if(pos_open_dt != 0 && last_tick.time >= pos_open_dt + HOLD_W * 60){
      
         Print("Closing position at ", last_tick.time);      
         CTrade trade;
         trade.PositionClose(_Symbol,100);
         //todo check close result
      }
   }
}


void OnDeinit(const int reason){

}
