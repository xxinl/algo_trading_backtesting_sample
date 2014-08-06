using System;

namespace BackTester
{
  public struct Tick
  {
    public DateTime Time;

    public string TimeStr;

    public double Ask;

    public double Bid;

    public double Last;

    public uint Volume;
  }

  public struct PerformanceTick
  {
    public DateTime Time;

    public double Last;

    public double Balance;

    public double Equity;

    public bool IsPosClosed;

    public int? CurrentSignal;

    public DateTime? CurrentPosOpenTime;

    public double? CurrentPosOpenRate;

    public bool IsBalanceUpdated;
  }
}
