
using System.Collections.Generic;
using System.Linq;
using GalaSoft.MvvmLight.Messaging;
using System;
using GalaSoft.MvvmLight.Threading;

namespace BackTester
{
  public class TickProcessor
  {
    private readonly int _leverage;
    private double _balance;
    private const int _lotSize = 5000; //0.05

    private double _posOpenRate = -1;
    private int? _currPosSignal = null;
    private DateTime? _posOpenTime = null;

    private List<double> _profits;
    private PerformanceSummary _performanceSummary;


    public TickProcessor(int leverage, double startBalance) {

      _leverage = leverage;
      _balance = startBalance;

      _performanceSummary = new PerformanceSummary();
      _profits = new List<double>();
    }

    public void OnTick(Tick tick, int signal, bool isClosePos)
    {
      PerformanceTick pTick = new PerformanceTick()
                              {
                                Time = tick.Time,
                                Last = tick.Last,
                                Balance = _balance,
                                Equity = _balance,
                                IsBalanceUpdated = false,
                                CurrentSignal = _currPosSignal,
                                CurrentPosOpenRate = _posOpenRate,
                                CurrentPosOpenTime = _posOpenTime,
                                IsPosClosed = false
                              };

      if (_currPosSignal.HasValue)
      {
        double profit = _calcProfit(_currPosSignal.Value, _lotSize, _posOpenRate,
          _currPosSignal == -1 ? tick.Ask : tick.Bid);
        double margin = _lotSize/(_currPosSignal == 1 ? _posOpenRate : 1)/_leverage;

        var equityOnTick = _balance + profit + margin;
        pTick.Equity = equityOnTick;

        //close current position
        if (isClosePos)
        {
          _balance = equityOnTick;
          pTick.Balance = _balance;
          pTick.IsBalanceUpdated = true;
          pTick.IsPosClosed = true;

          _profits.Add(profit);
          _updatePerformanceSummary(profit);
          DispatcherHelper.CheckBeginInvokeOnUI(() => Messenger.Default.Send(_performanceSummary));

          _currPosSignal = null;
          _posOpenRate = -1;
          _posOpenTime = null;
        }
      }
      else
      {
        //open new position
        if (Math.Abs(signal) == 1)
        {
          _posOpenRate = signal == 1 ? tick.Ask : tick.Bid;
          _posOpenTime = tick.Time;
          _currPosSignal = signal;

          double margin = _lotSize/(_currPosSignal == 1 ? _posOpenRate : 1)/_leverage;
          _balance = _balance - margin;
          pTick.Balance = _balance;
          pTick.IsBalanceUpdated = true;
        }
      }

      //only update graph each x hours
      if (pTick.Time.Minute == 0 || pTick.Time.Hour % 4 == 0) 
        DispatcherHelper.CheckBeginInvokeOnUI(() => Messenger.Default.Send(pTick));
    }

    private double _calcProfit(int signal, int lot, double openRate, double closeRate)
    {
      return signal * (closeRate - openRate) * lot;
    }

    private void _updatePerformanceSummary(double profit)
    {
      _performanceSummary.Profit += profit;
      if (profit >= 0)
      {
        _performanceSummary.TotalWin += profit;
        if (profit > _performanceSummary.MaxWin)
          _performanceSummary.MaxWin = profit;

        _performanceSummary.NoWinPos++;
        if (_currPosSignal.Value == 1)
          _performanceSummary.NoWinLong++;
        else
          _performanceSummary.NoWinShort++;
      }
      else
      {
        _performanceSummary.NoLossPos++;
        _performanceSummary.TotalLoss += profit;
        if (profit < _performanceSummary.MaxLoss)
          _performanceSummary.MaxLoss = profit;
      }

      _performanceSummary.SharpeR = _calcSharpe();
    }

    private double _calcSharpe()
    {
      double avg = _profits.Average();
      double sum = _profits.Sum(d => Math.Pow(d - avg, 2));
      return avg / Math.Sqrt(sum / _profits.Count());
    }
  }
}
