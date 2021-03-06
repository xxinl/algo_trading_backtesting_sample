
#include <Trade\Trade.mqh>

#import "strat.dll"

ulong get_dayrange_algo(string symbol, ulong complete_hour, double exit_lev, 
				ulong callback_handler);
ulong get_bollinger_algo(string symbol,
				ulong obser_win, double exit_lev, double ini_t, double obser_t, 
				ulong callback_handler);
int process_tick(ulong algo_p, string time, double ask, double bid, double last, ulong volume, 
						double stop_loss, bool &is_close_pos, double &risk,
						ulong callback_handler);
int delete_algo(ulong algo_p);

#import

enum AlgoType{

	//HYBRID = 0,
	DAYRANGE = 1,
	BOLLINGER = 2
};

//--- input parameters

input double SL = 0.01;
input double TP = 0.05;

input ulong COMPLETE_HOUR = 13;
input double EXIT_LEV = 0.00035;
input AlgoType ALGO_TYPE = BOLLINGER;

input ulong OBSER_WIN = 10;
input double EXIT_LEV_BL = 0.0002;
input double INI_T = 0.0004;
input double OBSER_T = 0.002;

//--- input parameters end

ulong algo_p = -1;

const int MAX_CLOSE_POS_RETRY = 5;

int OnInit(void){
   
   string symbol = _Symbol;
   
   switch(ALGO_TYPE){
   
   //case 0:
	  // algo_p = get_algo(StringSubstr(symbol, 0, 3), StringSubstr(symbol, 3, 3), ALGO_TYPE,
			//	 COMPLETE_HOUR, ENTRY_LEV, EXIT_LEV,
			//	 COMPLETE_HOUR, ENTRY_LEV, EXIT_LEV, 0);
		 //break;
	case 1:
	   algo_p = get_dayrange_algo(symbol, COMPLETE_HOUR,  EXIT_LEV, 0);
		break;
	case 2:
	   algo_p = get_bollinger_algo(symbol, 
				OBSER_WIN, EXIT_LEV_BL, INI_T, OBSER_T, 0);
		break;
   }
   
//   if(ALGO_TYPE == 0 || ALGO_TYPE == 1){
         
      MqlDateTime stm;
      MqlTick last_tick;
      SymbolInfoTick(_Symbol,last_tick);
      TimeToStruct(last_tick.time, stm);
      
      string start_str = IntegerToString(stm.year) + "." + IntegerToString(stm.mon) + "." + IntegerToString(stm.day);      
      datetime start = StringToTime(start_str);
      datetime end = start +  COMPLETE_HOUR * 60 * 60;
   
      MqlRates rt[];
      int count = CopyRates(_Symbol,_Period,start,end,rt);
      if(count > 0){
      
         for(int i = 0; i < count; i++){
      
            string dt_str =  TimeToString(rt[i].time, TIME_DATE|TIME_MINUTES|TIME_SECONDS);
            bool is_close_pos;
            double risk_lev;
            process_tick(algo_p, dt_str, rt[i].close, rt[i].close, rt[i].close, rt[i].tick_volume, 
                             SL, is_close_pos, risk_lev, 0);  
         }
      }
      else{
      
         Print("CopyRates of ",_Symbol," failed, no history");
         return(INIT_FAILED);
      }
 //  }   
   
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
   double risk_lev = 0;
   
   string dt_str;
   
   if(SymbolInfoTick(_Symbol,last_tick)){
   
      dt_str = TimeToString(last_tick.time, TIME_DATE|TIME_MINUTES|TIME_SECONDS);
      signal = process_tick(algo_p, dt_str, last_tick.ask, last_tick.bid, last_tick.last, last_tick.volume, 
                             SL, is_close_pos, risk_lev, 0);     
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
       
      double risk_dollar = _Symbol == "USDJPY" ? 30 * last_tick.last : 30;
       
      double dollar = risk_lev > 0 ? (int)(risk_dollar / (risk_lev * 4.5)) : 5000;	   
      double size = NormalizeDouble(dollar / 100000, 2);
   
      Print(_Symbol, ":opening position at ", last_tick.time, ". risk_lev: ", risk_lev, ". dollar: ", dollar, ". size: ", size);
	  
      CTrade trade;
      double price = SymbolInfoDouble(_Symbol, signal == 1 ? SYMBOL_ASK : SYMBOL_BID);
      bool open_result = trade.PositionOpen(_Symbol, signal == 1 ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, size,
											price, price - signal * SL, price + signal * TP);
	   if(open_result){	      
         Print(_Symbol, ":opened position at ", last_tick.time);
		   SendNotification(_Symbol + ":opened position at " + dt_str);
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
