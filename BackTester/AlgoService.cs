using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace BackTester
{
  public class AlgoService
  {
    #region dll imports

    [DllImport("strat.dll", EntryPoint = "get_algo", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr _get_algo(
      [MarshalAs(UnmanagedType.LPWStr)] string base_c,
      [MarshalAs(UnmanagedType.LPWStr)] string quote,
      [MarshalAs(UnmanagedType.LPWStr)] string path,
      _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "delete_algo", CallingConvention = CallingConvention.Cdecl)]
    private static extern int _delete_algo(IntPtr algo_add);

    [DllImport("strat.dll", EntryPoint = "process_tick", CallingConvention = CallingConvention.Cdecl)]
    private static extern int _process_tick(IntPtr algo_addr,
      [MarshalAs(UnmanagedType.LPWStr)] string time,
      double ask, double bid, double last, uint volume, double stop_loss,
      _callback _callbackInstance);

    [DllImport("strat.dll", EntryPoint = "optimize", CallingConvention = CallingConvention.Cdecl)]
    private static extern void _optimize(IntPtr algo_addr,
      [MarshalAs(UnmanagedType.LPWStr)] string path);

    [DllImport("strat.dll", EntryPoint = "process_end_day", CallingConvention = CallingConvention.Cdecl)]
    private static extern int _process_end_day(IntPtr algo_addr,
      [MarshalAs(UnmanagedType.LPWStr)] string hist_tick_path, uint keep_days_no);

    #endregion

    private IntPtr _algo_p;

    private _callback _callbackInstance;

    public delegate void _callback(string msg, int severity);

    public AlgoService(Action<DebugInfo> callbackInstance) {

      _callbackInstance = (msg, sev) => {

        if (callbackInstance != null) callbackInstance(new DebugInfo(msg, sev));
      };
    }

    ~AlgoService() {

      _delete_algo(_algo_p);
    }

    public async Task Init(string eventFilePath) {

      await Task.Run(() => {

        _algo_p = _get_algo("eur", "usd", eventFilePath, _callbackInstance);
      });
    }

    //todo stop loss
    public void OnTick(Tick t)
    {
      _process_tick(_algo_p, t.Time.ToString("yyyy.MM.dd HH:mm"), t.Ask, t.Bid, t.Last, t.Volume, 0.01,
        _callbackInstance);
    }
  }
}
