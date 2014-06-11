
namespace BackTester
{
  public class PerformanceSummary
  {
    public double Profit { get; set; }
    public int NoWinPos { get; set; }
    public int NoLossPos { get; set; }
    public int NoWinLong { get; set; }
    public int NoWinShort { get; set; }
    public double SharpeR { get; set; }

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
    public string NoWinLongDisp
    {
      get { return string.Format("{0}({1:P})", NoWinLong, (double)NoWinLong / NoWinPos); }
    }
    public string NoWinShortDisp
    {
      get { return string.Format("{0}({1:P})", NoWinShort, (double)NoWinShort / NoWinPos); }
    }
  }
}
