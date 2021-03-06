﻿using System;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace BackTester
{
  public class AlgoService : IDisposable
  {
    #region dll imports

    [DllImport("strat.dll", EntryPoint = "get_dayrange_algo", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern IntPtr _get_dayrange_algo(string symbol,
      IntPtr complete_hour, double exit_lev,
      _callback callback_handler);

    [DllImport("strat.dll", EntryPoint = "get_bollinger_algo", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern IntPtr _get_bollinger_algo(string symbol,
      IntPtr obser_win, double exit_lev, double ini_t, double obser_t,
      _callback callback_handler);

    [DllImport("strat.dll", EntryPoint = "get_hybrid_algo", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern IntPtr _get_hybrid_algo(string symbol, IntPtr[] algos,
      _callback callback_handler);

    [DllImport("strat.dll", EntryPoint = "delete_algo", CallingConvention = CallingConvention.Cdecl)]
    private static extern int _delete_algo(IntPtr algo_add);

    [DllImport("strat.dll", EntryPoint = "process_tick", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern int _process_tick(IntPtr algo_addr, string time,
      double ask, double bid, double last, IntPtr volume, double stop_loss,
      [Out] out bool is_close_pos, [Out] out double risk, _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "optimize", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern void _optimize(IntPtr algo_addr, string path, string start_date, string end_date,
      IntPtr max_iteration, IntPtr population_size,
      _callback _callbackInstance);

    //[DllImport("strat.dll", EntryPoint = "process_end_day", CallingConvention = CallingConvention.Cdecl,
    //  CharSet = CharSet.Unicode)]
    //private static extern int _process_end_day(IntPtr algo_addr, string hist_tick_path, IntPtr keep_days_no,
    //  _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "reset_algo_params", CallingConvention = CallingConvention.Cdecl)]
    private static extern void _reset_algo_params(IntPtr algo_addr);

    #endregion

    private bool _disposed = false;
    private IntPtr _algo_p;
    //private string _eventFilePath;
    private string _tickFilePath;

    private Action<DebugInfo> _uiCallbackAction;
    private _callback _callbackInstance;
    public delegate void _callback(string msg, int severity);
    

    public AlgoService(Action<DebugInfo> callbackInstance, string tickFilePath)
    {
      _uiCallbackAction = callbackInstance;
      _tickFilePath = tickFilePath;

      _callbackInstance = (msg, sev) =>
      {
        if (callbackInstance != null) callbackInstance(
          new DebugInfo(string.Format("{0} {1}", DateTime.Now.ToString("T"), msg), sev));
      };
    }

    public async Task InitDayRange(int completeHour, double exitLev)
    {
      await Task.Run(() =>
                     {
                       _algo_p = _get_dayrange_algo("xxx",
                         (IntPtr) completeHour, exitLev,
                         _callbackInstance);
                     });
    }

    public async Task InitBollinger(int obserWin, double exitLev, double iniT, double obserT)
    {
      await Task.Run(() =>
      {
        _algo_p = _get_bollinger_algo("xxx",
          (IntPtr)obserWin, exitLev, iniT, obserT,
          _callbackInstance);
      });
    }

    public async Task InitHybrid(int completeHour, double exitLev, int obserWin, 
      double exitLev2, double iniT, double obserT)
    {
      await Task.Run(() =>
      {
        IntPtr[] algos = new IntPtr[2];
        algos[0] = _get_dayrange_algo("xxx", (IntPtr)completeHour, exitLev, _callbackInstance);
        algos[1] = _get_bollinger_algo("xxx", (IntPtr)obserWin, exitLev2, iniT, obserT, _callbackInstance);

        _algo_p = _get_hybrid_algo("xxx", algos, _callbackInstance);
      });
    }

    public int OnTick(Tick t, out bool isClosePos, out double risk, double sl)
    {
      return _process_tick(_algo_p, t.TimeStr, t.Ask, t.Bid, t.Last,
        (IntPtr)t.Volume, sl, out isClosePos, out risk,
        _callbackInstance);
    }

    //public async Task Optimize(DateTime tickDate, int algoType, 
    //  int completeHour, double entryLev, double exitLev,
    //  int completeHour2, double entryLev2, double exitLev2,
    //  int backNoofDays, int maxIteration = 32, int populationSize = 128)
    //{
    //  using (AlgoService optiAlgo = new AlgoService(_uiCallbackAction, _tickFilePath))
    //  {
    //    await optiAlgo.Init(
    //      algoType, completeHour, entryLev, exitLev,
    //      completeHour2, entryLev2, exitLev2);

    //    await optiAlgo.OptimizeExecute(tickDate, backNoofDays, maxIteration, populationSize);
    //  }
    //}

    public async Task Optimize(DateTime tickDate, int backNoofDays, int maxIteration, int populationSize)
    {
      await Task.Run(() =>
                     {
                       string startDateStr = tickDate.AddDays(0-backNoofDays).ToString("yyyy.MM.dd HH:mm");
                       string endDateStr = tickDate.ToString("yyyy.MM.dd HH:mm");
                       _optimize(_algo_p, _tickFilePath, startDateStr, endDateStr,
                         (IntPtr)maxIteration, (IntPtr)populationSize, _callbackInstance);
                     });
    }

    public void ResetAlgoParams()
    {
      _reset_algo_params(_algo_p);
    }

    #region IDisposable

   // Public implementation of Dispose pattern callable by consumers. 
   public void Dispose()
   { 
      Dispose(true);
      GC.SuppressFinalize(this);           
   }

   // Protected implementation of Dispose pattern. 
   protected virtual void Dispose(bool disposing)
   {
      if (_disposed)
         return; 

      if (disposing) {
         // Free any other managed objects here. 
         //
      }

      // Free any unmanaged objects here. 
      _delete_algo(_algo_p);

      _disposed = true;
   }

   ~AlgoService()
   {
     Dispose(false);
   }

    #endregion
  }
}
