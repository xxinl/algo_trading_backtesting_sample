using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using GalaSoft.MvvmLight.Command;
using GalaSoft.MvvmLight.Threading;
using GalaSoft.MvvmLight.Messaging;
using System.Threading;
using BackTester.Views;

namespace BackTester.ViewModels
{
  public class BackTesterViewModel : GalaSoft.MvvmLight.ViewModelBase
  {
    private CancellationTokenSource _cts;
    private DebugInfoWin _debugInfoWin;
    private SummaryWin _summaryWin;

    #region properties

    #region input properties

    public DateTime StartDate { get; set; }
    public DateTime EndDate { get; set; }
    public string TickFilePath { get; set; }
    //public string EventFilePath { get; set; }
    public int Leverage { get; set; }
    public double StartBalance { get; set; }
    public bool RunTestWithOptimizer { get; set; }
    public int ObserWin { get; set; }
    public double ExitLev2 { get; set; }
    public double Threshold1 { get; set; }
    public double Threshold2 { get; set; }
    public int OptimizeInterval { get; set; } //days
    public int OptimizeLookback { get; set; } //days
    public double SL { get; set; }
    public int CompleteHour { get; set; }
    public double ExitLev { get; set; }
    //public double ExtendFactor { get; set; }
    //public int CompleteHour2 { get; set; }
    //public double EntryLev2 { get; set; }
    //public double ExitLev2 { get; set; }
    public int AlgoType { get; set; }

    #endregion input properties

    private int _progressValue;
    public int ProgressValue
    {
      get { return _progressValue; }
      set { _progressValue = value; this.RaisePropertyChanged(() => this.ProgressValue); }
    }

    public string RunState { get; set; }

    private bool _isBusy;
    public bool IsBusy
    {
      get { return _isBusy; }
      set
      {
        _isBusy = value;
        this.RaisePropertyChanged(() => this.IsBusy);
        RunState = _isBusy ? "Cancel" : "Run";
        this.RaisePropertyChanged(() => this.RunState);
      }
    }

    private bool _showDebugWin = false;

    public bool ShowDebugWin
    {
      get { return _showDebugWin; }
      set { _showDebugWin = value; }
    }


    public RelayCommand RunTestCommand { get; private set; }
    public RelayCommand RunOptimizeCommand { get; private set; }

    #endregion properties
    
    public BackTesterViewModel() {

      StartDate = new DateTime(2013,01,01);
      EndDate = new DateTime(2013, 12, 31);

      TickFilePath = @"C:\workspace\Strat\back_test_files\EURUSD2013.csv";
      //EventFilePath = @"C:\workspace\Strat\back_test_files\Calendar-2013.csv";
      RunState = "Run";
      Leverage = 500;
      StartBalance = 0;
      RunTestWithOptimizer = false;
      ObserWin = 10;
      ExitLev2 = 0.00025;
      Threshold1 = 0.0004;
      Threshold2 = 0.002;
      OptimizeInterval = 30;
      OptimizeLookback = 90;
      SL = 0.0055;
      CompleteHour = 13;
      ExitLev = 0.00025;
      //ExtendFactor = 1.5;
      //CompleteHour2 = 15;
      //EntryLev2 = 0.0001;
      //ExitLev2 = 0.0002;
      AlgoType = 2;

      RunTestCommand = new RelayCommand(async () => {
        if (IsBusy)
          _cancelTest();
        else
        {
          _cts = new CancellationTokenSource();
          await _runTest(_cts.Token);
        }
      });

      RunOptimizeCommand = new RelayCommand(async () =>
      {
        _cts = new CancellationTokenSource();
        await _runOptimize(_cts.Token);
      });
    }

    private async Task _runTest(CancellationToken ct) {

      IsBusy = true;

      TickProcessor tickPro = new TickProcessor(Leverage, StartBalance);

      if (_summaryWin == null)
      {
        _summaryWin = new SummaryWin();
      }
      else
      {
        _summaryWin.Reset();
      }

      _summaryWin.Show();
      
      try {

        _setProgress();
        using (AlgoService algo = new AlgoService(_onMessage, TickFilePath))
        {
          switch (AlgoType)
          {
            case 0:
              break;
            case 1:
              await algo.InitDayRange(CompleteHour, ExitLev);
              break;
            case 2:
              await algo.InitBollinger(ObserWin, ExitLev2, Threshold1, Threshold2);
              break;
          }

          _setProgress();
          List<Tick> ticks = await Util.ReadTickCsv(TickFilePath, StartDate, EndDate, ct);
          await _runTicks(ticks, algo, tickPro, ct);
        }
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
          }
          else
          {
            if (ShowDebugWin)
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

    private async Task _runTicks(List<Tick> ticks, AlgoService algo, TickProcessor tickPro, 
      CancellationToken ct)
    {
      await Task.Factory.StartNew(() =>
                                  {
                                    var lastOptimizeDate = StartDate;

                                    for (int i = 0; i < ticks.Count; i++)
                                    {
                                      if (ct.IsCancellationRequested == true)
                                      {
                                        ct.ThrowIfCancellationRequested();
                                      }

                                      Tick tick = ticks[i];

                                      //if (RunTestWithOptimizer &&
                                      //    tick.Time.Date.AddDays(0 - OptimizeInterval) > lastOptimizeDate &&
                                      //    tick.Time.Date.AddDays(0 - OptimizeLookback) > StartDate)
                                      //{
                                      //  //TODO new algo instance need to be created here
                                      //  //algo.Optimize(tick.Time.Date, AlgoType,
                                      //  //  CompleteHour, EntryLev, ExitLev,
                                      //  //  CompleteHour2, EntryLev2, ExitLev2, OptimizeLookback).Wait(ct);

                                      //  algo.ResetAlgoParams();
                                      //  lastOptimizeDate = tick.Time.Date;
                                      //}

                                      bool isClosePos = false;
                                      double riskLev = -1;
                                      var signal = algo.OnTick(tick, out isClosePos, out riskLev, SL);
                                      tickPro.OnTick(tick, signal, isClosePos, riskLev);

                                      if(i % 100 == 0)
                                        _setProgress(i*100/ticks.Count);
                                    }

                                    _setProgress(100);
                                  }, ct);
    }

    private async Task _runOptimize(CancellationToken ct)
    {
      IsBusy = true;

      try
      {
        _setProgress();
        using (AlgoService algo = new AlgoService(_onMessage, TickFilePath))
        {
          switch (AlgoType)
          {
            case 0:
              break;
            case 1:
              await algo.InitDayRange(CompleteHour, ExitLev);
              break;
            case 2:
              await algo.InitBollinger(ObserWin, ExitLev2, Threshold1, Threshold2);
              break;
          }

          int backNoDays = Convert.ToInt32((EndDate.Date - StartDate.Date).TotalDays);
          await algo.Optimize(EndDate, backNoDays, 128, 512);
        }
      }
      catch (OperationCanceledException)
      {
        //
      }
      catch (Exception e)
      {

        throw;
      }
      finally
      {

        IsBusy = false;
      }
    }
  }
}
