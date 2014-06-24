
#include <Trade\Trade.mqh>

#import "strat.dll"
ulong get_algo(string base, string quote, ulong obser_win, ulong hold_win, double ini_t, double obser_t,
									ulong callback_handler);
int process_tick(ulong algo_p, string time, double ask, double bid, double last, ulong volume, 
									double stop_loss, bool &is_close_pos, ulong callback_handler);
int delete_algo(ulong algo_p);
#import

//--- input parameters
input ulong OBSER_WIN = 1;
input ulong HOLD_WIN = 1;
input double INI_T = 0.00075;
input double OBSER_T = 0.001;

ulong algo_p = -1;

int OnInit(void){
   
   algo_p = get_algo("eur", "usd", OBSER_WIN, HOLD_WIN, INI_T, OBSER_T, 0);
   
   if(algo_p == -1){
   
      Print("Failed to create algo object");
      return(INIT_FAILED);
   }
     
   return(INIT_SUCCEEDED);
 }

void OnTick(void){
     
   MqlTick last_tick;
   int signal = 0;
   bool is_close_pos = false;
   
   if(SymbolInfoTick(_Symbol,last_tick)){
   
      string dt_str =  TimeToString(last_tick.time, TIME_DATE|TIME_MINUTES);
      signal = process_tick(algo_p, dt_str, last_tick.ask, last_tick.bid, last_tick.last, last_tick.volume, 
                              0.01, is_close_pos, 0);     
   }
   else{
   
      Print("OnTick:SymbolInfoTick() failed, error = ",GetLastError());
      return;
   }   
   
   if(signal != 0 && !PositionSelect(_Symbol)){
      
      MqlDateTime stm;
      TimeToStruct(last_tick.time,stm);
      
      if(stm.hour >= 22 || stm.hour <=7){
      
         return;
      }
      
      double size = 0.05;
   
      Print("Opening position at ", last_tick.time);
      CTrade trade;
      double price = SymbolInfoDouble(_Symbol, signal == 1 ? SYMBOL_ASK : SYMBOL_BID);
      trade.PositionOpen(_Symbol, signal == 1 ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, size,
                         price, price - signal * 0.01, price + signal * 0.05);
   }
   
   if(PositionSelect(_Symbol) && is_close_pos){
      
      Print("Closing position at ", last_tick.time);      
      CTrade trade;
      trade.PositionClose(_Symbol,100);
      //todo check close result
   }
}


void OnDeinit(const int reason){

   delete_algo(algo_p);
}
