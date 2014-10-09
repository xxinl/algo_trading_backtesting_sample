using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Web;
using Tradedash.Models;

namespace Tradedash.Dal
{
  public class TradeRepository
  {
    private readonly string _tddbconn;

    public TradeRepository()
    {
      _tddbconn = ConfigurationManager.ConnectionStrings["TDDB"].ToString();
    }

    public void SaveTrade(Trade td)
    {
      using (SqlConnection cnn = new SqlConnection(_tddbconn))
      {
        cnn.Open();

        string proc = "[trade_insert]";

        using (SqlCommand cmd = new SqlCommand(proc, cnn))
        {
          cmd.CommandType = CommandType.StoredProcedure;
          cmd.Parameters.AddWithValue("@time", td.Time);
          cmd.Parameters.AddWithValue("@symbol", td.Symbol);
          cmd.Parameters.AddWithValue("@type", td.Type == "Buy");
          cmd.Parameters.AddWithValue("@direction", td.Direction == "In");
          cmd.Parameters.AddWithValue("@volume", td.Volume);
          cmd.Parameters.AddWithValue("@price", td.Price);
          cmd.Parameters.AddWithValue("@commission", td.Commission.HasValue ? td.Commission : (object)DBNull.Value);
          cmd.Parameters.AddWithValue("@swap", td.Swap.HasValue ? td.Swap : (object)DBNull.Value);
          cmd.Parameters.AddWithValue("@profit", td.Profit);
          cmd.Parameters.AddWithValue("@comment", td.Comment != null ? td.Commission : (object)DBNull.Value);
          cmd.Parameters.AddWithValue("@acc_no", td.AccNo);

          try
          {
            cmd.ExecuteNonQuery();
          }
          catch (Exception ex)
          {
            throw ex;
          }
        }
      }
    }

    public List<Trade> Search(DateTime startDate, DateTime endDate, int accId)
    {
      using (SqlConnection cnn = new SqlConnection(_tddbconn))
      {
        cnn.Open();

        string proc = "[trade_search]";

        List<Trade> trades = new List<Trade>();

        using (SqlCommand cmd = new SqlCommand(proc, cnn))
        {
          cmd.CommandType = CommandType.StoredProcedure;
          cmd.Parameters.AddWithValue("@time_start", startDate);
          cmd.Parameters.AddWithValue("@time_end", endDate);
          cmd.Parameters.AddWithValue("@acc_id", accId);

          try
          {
            SqlDataReader dr = cmd.ExecuteReader(CommandBehavior.CloseConnection);

            while (dr.Read())
            {
              Trade td = new Trade
                            {
                              Id = (int)dr["Id"],
                              Time = (DateTime)dr["time"],
                              Symbol = (string)dr["symbol"],
                              Type = (bool)dr["type"] ? "Buy" : "Sell",
                              Direction = (bool)dr["direction"] ? "In" : "Out",
                              Volume = (decimal)dr["volume"],
                              Price = (decimal)dr["price"],
                              Commission = dr["commission"] is DBNull ? (decimal?)null : (decimal)dr["commission"],
                              Swap = dr["swap"] is DBNull ? (decimal?)null : (decimal)dr["swap"],
                              Profit = (decimal)dr["profit"],
                              Comment = dr["comment"] is DBNull ? null : (string)dr["comment"],
                              AccNo = (string)dr["acc_no"]
                            };
              trades.Add(td);
            }

            return trades;
          }
          catch (Exception ex)
          {
            throw ex;
          }
        }
      }
    }
  }
}