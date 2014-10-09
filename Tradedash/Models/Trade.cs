using System;

namespace Tradedash.Models
{
  public class Trade
  {
    public int? Id { get; set; }
    public DateTime Time { get; set; }
    public string Symbol { get; set; }
    public string Type { get; set; }
    public string Direction { get; set; }
    public decimal Volume { get; set; }
    public decimal Price { get; set; }
    public decimal? Commission { get; set; }
    public decimal? Swap { get; set; }
    public decimal Profit { get; set; }
    public string Comment { get; set; }
    public string AccNo { get; set; }

    public Trade()
    {
      Id = null;
    }
  }
}