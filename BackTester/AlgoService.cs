using System;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace BackTester
{
  public class AlgoService : IDisposable
  {
    #region dll imports

    [DllImport("strat.dll", EntryPoint = "get_algo", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern IntPtr _get_algo(
      string base_c, string quote, 
      //string path,
      IntPtr obser_win, IntPtr hold_win, double ini_t, double obser_t,
      _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "delete_algo", CallingConvention = CallingConvention.Cdecl)]
    private static extern int _delete_algo(IntPtr algo_add);

    [DllImport("strat.dll", EntryPoint = "process_tick", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern int _process_tick(IntPtr algo_addr,
      string time, double ask, double bid, double last, IntPtr volume, double stop_loss, [Out] out bool is_close_pos,
      _callback _callbackInstance);

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

    public async Task Init(//string eventFilePath, 
      int obserWin, int holdWin, double t1, double t2)
    {
      //_eventFilePath = eventFilePath;

      await Task.Run(() =>
      {
        _algo_p = _get_algo("eur", "usd", 
          //eventFilePath,
          (IntPtr)obserWin, (IntPtr)holdWin, t1, t2, _callbackInstance);
      });
    }

    //todo stop loss
    public int OnTick(Tick t, out bool isClosePos)
    {
      return _process_tick(_algo_p, t.Time.ToString("yyyy.MM.dd HH:mm"), t.Ask, t.Bid, t.Last, 
        (IntPtr)t.Volume, 0.01, out isClosePos,
        _callbackInstance);
    }

    public async Task Optimize(DateTime tickDate, int obserWin, int holdWin, double t1, double t2,
      int backNoofDays, int maxIteration = 32, int populationSize = 128)
    {
      using (AlgoService optiAlgo = new AlgoService(_uiCallbackAction, _tickFilePath))
      {
        await optiAlgo.Init(
          //_eventFilePath, 
          obserWin, holdWin, t1, t2);

        await optiAlgo.OptimizeExecute(tickDate, backNoofDays, maxIteration, populationSize);
      }
    }

    public async Task OptimizeExecute(DateTime tickDate, int backNoofDays, int maxIteration, int populationSize)
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
