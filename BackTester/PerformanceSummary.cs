
namespace BackTester
{
  public class PerformanceSummary
  {
    private double _maxProfit;
    private double _currentDranDown;

    private double _profit;
    public double Profit
    {
      get { return _profit; }
      set
      {
        _profit = value;
        if (_profit > _maxProfit)
        {
          _maxProfit = _profit;
          _currentDranDown = 0;
        }
        else
        {
          _currentDranDown = _maxProfit - _profit;
          if (_currentDranDown > MaxDrawDown)
            MaxDrawDown = _currentDranDown;
        }
      }
    }

    public int NoWinPos { get; set; }
    public int NoLossPos { get; set; }
    public int NoLong { get; set; }
    public int NoShort { get; set; }
    public int NoWinLong { get; set; }
    public int NoWinShort { get; set; }
    public double SharpeR { get; set; }
    public double TotalWin { get; set; }
    public double TotalLoss { get; set; }
    public double MaxWin { get; set; }
    public double MaxLoss { get; set; }
    public double MaxDrawDown { get; set; }

    public int TotalNoPos 
    {
      get { return NoWinPos + NoLossPos; }
    }

    public string NoWinDisp
    {
      get { return string.Format("{0}({1:P})", NoWinPos, (double)NoWinPos / TotalNoPos); }
    }
    public string NoLossDisp
    {
      get { return string.Format("{0}({1:P})", NoLossPos, (double)NoLossPos / TotalNoPos); }
    }
    public string NoLongDisp
    {
      get { return string.Format("{0}({1:P})", NoLong, (double)NoWinLong / NoLong); }
    }
    public string NoShortDisp
    {
      get { return string.Format("{0}({1:P})", NoShort, (double)NoWinShort / NoShort); }
    }
    public double AvgWin {
      get { return TotalWin / NoWinPos; }
    }
    public double AvgLoss {
      get { return TotalLoss / NoLossPos; }
    }
  }
}
