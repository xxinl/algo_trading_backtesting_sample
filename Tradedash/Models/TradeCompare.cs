using System;
using System.Collections.Generic;
using System.Linq;

namespace Tradedash.Models
{
  public class TradeCompare
  {
    public List<AccountInfo> AccInfos { get; set; }

    //time series of trades of each acc
    public List<List<Trade>> TradesTimeLine { get; set; }

    public TradeCompare()
    {
      AccInfos = new List<AccountInfo>();
      TradesTimeLine = new List<List<Trade>>();
    }

    public void CalculatePerformance()
    {
      for(int i = 0; i < AccInfos.Count; i++)
      {
        var profits = TradesTimeLine.Select(tl => Convert.ToDouble(tl[i].Profit))
          .Where(t => t > 0).ToList();

        AccInfos[i].SharpeR = _calcSharpe(profits);
      }
    }

    private double _calcSharpe(List<double> profits)
    {
      double avg = profits.Average();
      double sum = profits.Sum(d => Math.Pow(d - avg, 2));
      return avg / Math.Sqrt(sum / profits.Count());
    }
  }

  public class AccountInfo
  {
    public string AccountNo { get; set; }
    public double SharpeR { get; set; }
  }
}