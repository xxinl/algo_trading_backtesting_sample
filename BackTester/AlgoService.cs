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
      string base_c, string quote, string path, _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "delete_algo", CallingConvention = CallingConvention.Cdecl)]
    private static extern int _delete_algo(IntPtr algo_add);

    [DllImport("strat.dll", EntryPoint = "process_tick", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern int _process_tick(IntPtr algo_addr,
      string time, double ask, double bid, double last, IntPtr volume, double stop_loss,
      _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "optimize", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern void _optimize(IntPtr algo_addr, string path);

    [DllImport("strat.dll", EntryPoint = "process_end_day", CallingConvention = CallingConvention.Cdecl,
      CharSet = CharSet.Unicode)]
    private static extern int _process_end_day(IntPtr algo_addr, string hist_tick_path, IntPtr keep_days_no);

    #endregion

    private bool _disposed = false;

    private IntPtr _algo_p;

    private _callback _callbackInstance;

    public delegate void _callback(string msg, int severity);

    public AlgoService(Action<DebugInfo> callbackInstance)
    {
      _callbackInstance = (msg, sev) =>
      {
        if (callbackInstance != null) callbackInstance(new DebugInfo(msg, sev));
      };
    }

    public async Task Init(string eventFilePath)
    {
      await Task.Run(() =>
      {
        _algo_p = _get_algo("eur", "usd", eventFilePath, _callbackInstance);
      });
    }

    //todo stop loss
    public int OnTick(Tick t)
    {
      return _process_tick(_algo_p, t.Time.ToString("yyyy.MM.dd HH:mm"), t.Ask, t.Bid, t.Last, (IntPtr)t.Volume, 0.01,
        _callbackInstance);
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
