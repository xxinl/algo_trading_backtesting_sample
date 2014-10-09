using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Http;
using Tradedash.Dal;
using Tradedash.Models;

namespace Tradedash.Controllers
{
  public class TradeController : ApiController
  {
    [HttpGet, Route("api/gettradecompare/{startDate:datetime}/{endDate:datetime}")]
    public IHttpActionResult GetTradeCompare(DateTime startDate, DateTime endDate)
    {
      try
      {
        TradeCompare tc = new TradeCompare();

        var repo = new TradeRepository();
        List<Trade> trades = repo.Search(startDate, endDate, 1);
        tc = _getTradeCompare(tc, trades);

        return Ok(tc);
      }
      catch (Exception ex)
      {
        return InternalServerError(ex);
      }
    }

    private TradeCompare _getTradeCompare(TradeCompare tc, List<Trade> trades)
    {
      trades = trades.Where(t => t.Symbol == "EURUSD" && t.Direction == "Out").ToList();

      //build account list
      foreach (var trade in trades)
      {
        if (tc.AccInfos.All(acc => acc.AccountNo != trade.AccNo))
          tc.AccInfos.Add(new AccountInfo() {AccountNo = trade.AccNo});
      }

      Trade lastTrade = null;
      List<Trade> tradesTimeLineNodeWithPlaceHolder = null;
      int crrAccIndex = 0;

      //loop trades, assume already ordered in time decending and account ascending
      foreach (var trade in trades)
      {
        //start a new time-line row
        if (lastTrade == null || trade.Time < lastTrade.Time.AddSeconds(-60) 
          || trade.Symbol != lastTrade.Symbol || trade.Type != lastTrade.Type 
          || trade.Direction != lastTrade.Direction)
        {
          if (tradesTimeLineNodeWithPlaceHolder != null)
          {
            //fill empty trades for place holder
            for (; crrAccIndex < tc.AccInfos.Count; crrAccIndex++)
            {
              tradesTimeLineNodeWithPlaceHolder.Add(new Trade());
            }

            tc.TradesTimeLine.Add(tradesTimeLineNodeWithPlaceHolder);
          }

          tradesTimeLineNodeWithPlaceHolder = new List<Trade>();
          crrAccIndex = 0;
        }

        //fill empty trades for place holder
        for (; crrAccIndex < tc.AccInfos.Count; crrAccIndex++)
        {
          if (tc.AccInfos[crrAccIndex].AccountNo == trade.AccNo)
          {
            tradesTimeLineNodeWithPlaceHolder.Add(trade);
            crrAccIndex++;
            break;
          }
          else
          {
            tradesTimeLineNodeWithPlaceHolder.Add(new Trade()
            {
              Symbol = trade.Symbol,
              Time = trade.Time,
              Direction = trade.Direction,
              Type = trade.Type
            });
          }
        }

        lastTrade = trade;
      }

      tc.CalculatePerformance();

      return tc;
    }
  }
}
