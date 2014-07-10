
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
input double SL = 0.01;
input double TP = 0.05;

ulong algo_p = -1;

const int MAX_CLOSE_POS_RETRY = 5;

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
   
   string dt_str;
   
   if(SymbolInfoTick(_Symbol,last_tick)){
   
      dt_str =  TimeToString(last_tick.time, TIME_DATE|TIME_MINUTES);
      signal = process_tick(algo_p, dt_str, last_tick.ask, last_tick.bid, last_tick.last, last_tick.volume, 
                             -1/*SL*/, is_close_pos, 0);     
   }
   else{
   
      Print("OnTick:SymbolInfoTick() failed, error = ",GetLastError());
      return;
   }   
   
   bool has_position = PositionSelect(_Symbol);
   
   if(has_position && is_close_pos){
      
		ClosePosition(dt_str);
   }
   
   if(signal != 0 && !has_position){
      
      MqlDateTime stm;
      TimeToStruct(last_tick.time,stm);
     
      double size = 0.05;
   
      Print(_Symbol, ":opening position at ", last_tick.time);
	  
      CTrade trade;
      double price = SymbolInfoDouble(_Symbol, signal == 1 ? SYMBOL_ASK : SYMBOL_BID);
      bool open_result = trade.PositionOpen(_Symbol, signal == 1 ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, size,
											price, price - signal * SL, price + signal * TP);
	   if(open_result){	      
         Print(_Symbol, ":opened position at ", last_tick.time);
      }
      else {
         //--- failure message
         Print(_Symbol, ":PositionOpen() method failed. Return code=",trade.ResultRetcode(),
         	". Code description: ",trade.ResultRetcodeDescription());
      }
   }
}

void OnDeinit(const int reason){

	if(PositionSelect(_Symbol)){
	
		Print(_Symbol, ":closing open position before exit");   
		ClosePosition("Exit");
	}

   delete_algo(algo_p);
}

void ClosePosition(string dt_str){

	Print(_Symbol, ":closing position at ", dt_str);   

	CTrade trade;
	
	bool succeed = false;
	for(int i = 0; i < MAX_CLOSE_POS_RETRY; i++){
	
		if(trade.PositionClose(_Symbol)) {
	  
			Print(_Symbol, ":Closed position at ", dt_str);
			SendNotification(_Symbol + ":Closed position at " + dt_str);
			 
			succeed = true;
			break;
		}
		else{
		
			Print(_Symbol, ":PositionClose() method failed. Return code=",trade.ResultRetcode(),
				". Code description: ",trade.ResultRetcodeDescription());
		}
	}
	
	if(!succeed){
	
      Print(_Symbol, ":PositionClose() method failed after 5 attempts");
      
      SendNotification(_Symbol + ":Failed to close position after 5 attempts, manual operation needed.");
	}
}
