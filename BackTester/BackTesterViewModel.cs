using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GalaSoft.MvvmLight.Command;
using GalaSoft.MvvmLight.Threading;
using GalaSoft.MvvmLight.Messaging;
using System.Threading;
using System.Windows;

namespace BackTester
{
  public class BackTesterViewModel : GalaSoft.MvvmLight.ViewModelBase
  {
    private CancellationTokenSource _cts;
    private DebugInfoWin _debugInfoWin;

    #region properties

    public DateTime StartDate { get; set; }
    public DateTime EndDate { get; set; }
    public string TickFilePath { get; set; }
    public string EventFilePath { get; set; }

    private int _progressValue;
    public int ProgressValue
    {
      get { return _progressValue; }
      set { _progressValue = value; this.RaisePropertyChanged(() => this.ProgressValue); }
    }
    public string RunState { get; set; }

    public RelayCommand RunTestCommand { get; private set; }

    private bool _isBusy;
    public bool IsBusy {
      get { return _isBusy; }
      set { 
        _isBusy = value;
        this.RaisePropertyChanged(() => this.IsBusy);
        RunState = _isBusy ? "Cancel" : "Run";
        this.RaisePropertyChanged(() => this.RunState);
      }
    }

    #endregion

    public BackTesterViewModel() {
    
      StartDate = new DateTime(2013,01,01);
      EndDate = new DateTime(2013, 01, 31);

      TickFilePath = @"C:\workspace\Strat\back_test_files\EURUSD_2013_1min_alpari.csv";
      EventFilePath = @"C:\workspace\Strat\back_test_files\Calendar-Jan2013.csv";
      RunState = "Run";

      RunTestCommand = new RelayCommand(async () => {
        if (IsBusy)
          _cancelTest();
        else
        {
          _cts = new CancellationTokenSource();
          await _runTest(_cts.Token);
        }
      });
    }

    private async Task _runTest(CancellationToken ct) {

      IsBusy = true;

      try {

        _setProgress();
        AlgoService algo = new AlgoService(_onMessage);
        await algo.Init(EventFilePath);

        _setProgress();
        List<Tick> ticks = await Util.ReadTickCsv(TickFilePath, StartDate, EndDate);
        await _runTicks(ticks, algo);
      }
      catch (OperationCanceledException)
      {
        //
      }
      catch(Exception e) {

        throw;
      }
      finally{
      
        IsBusy = false;
      }
    }

    private void _cancelTest() {

      if (_cts != null)
      {
        _cts.Cancel();
      }

      IsBusy = false;
      _setProgress();
    }

    private void _onMessage(DebugInfo info) {
      
      DispatcherHelper.CheckBeginInvokeOnUI(() =>
      {
        if (info != null && !string.IsNullOrWhiteSpace(info.Info))
        {
          if (_debugInfoWin == null)
          {
            _debugInfoWin = new DebugInfoWin();
            _debugInfoWin.Show();
          }

          Messenger.Default.Send(info);
        }
      });
    }

    private void _setProgress(int i = 0) {

      DispatcherHelper.CheckBeginInvokeOnUI(() =>
      {
        ProgressValue = i;
      });
    }

    private async Task _runTicks(List<Tick> ticks, AlgoService algo)
    {
      await Task.Run(() =>
      {
        for (int i = 0; i < ticks.Count; i++)
        {
          algo.OnTick(ticks[i]);
          _setProgress(i * 100 / ticks.Count);
        }
      });
    }
  }
}
