using GalaSoft.MvvmLight.Messaging;
using System;
using System.Collections.Generic;
using GalaSoft.MvvmLight.Threading;

namespace BackTester
{
  public class TickProcessor
  {
    private readonly int _leverage;
    private double _balance;
    private const int _lotSize = 1000; //0.01

    private double _posOpenRate = -1;
    private int? _currPosSignal = null;

    private List<PerformanceTick> _perTicks;

    public TickProcessor(int leverage, double startBalance) {

      _leverage = leverage;
      _balance = startBalance;

      _perTicks = new List<PerformanceTick>();
    }

    public void OnTick(Tick tick, int signal) {

      PerformanceTick pTick = new PerformanceTick()
      {
        Time = tick.Time,
        Last = tick.Last,
        Balance = _balance,
        Equity = _balance
      };

      if (Math.Abs(signal) == 1)
      {
        _posOpenRate = signal == 1 ? tick.Ask : tick.Bid;
        _currPosSignal = signal;

        var margin = _lotSize / (signal == 1 ? _posOpenRate : 1) / _leverage;
        pTick.Equity = _balance;
        _balance = _balance - margin;
        pTick.Balance = _balance;
      }
      else
      {
        if(_currPosSignal != null)
        {
          var profit = _calcProfit(_currPosSignal.Value, _lotSize, _posOpenRate, _currPosSignal == -1 ? tick.Ask : tick.Bid);
          var margin = _lotSize / (_currPosSignal == 1 ? _posOpenRate : 1) / _leverage;

          if (signal == 2)
          {
            _balance = _balance + profit + margin;
            pTick.Equity = _balance;
            pTick.Balance = _balance;
            _currPosSignal = null;
          }
          else
            pTick.Equity = _balance + profit + margin;
        }
      }

      DispatcherHelper.CheckBeginInvokeOnUI(() => Messenger.Default.Send(pTick));
      
      _perTicks.Add(pTick);
    }

    //assume home cuurency is quote currency
    private double _calcProfit(int signal, int lot, double openRate, double closeRate)
    {
      return signal * (closeRate - openRate) * lot / (signal == 1 ? closeRate : 1);
    }
  }
}
