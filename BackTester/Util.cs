using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Globalization;
using System.Threading;

namespace BackTester
{
  public class Util
  {
    //cols: column filter, 0 based index
    public static async Task<List<Tick>> ReadTickCsv(string path, DateTime startDate, DateTime endDate, 
      CancellationToken ct) {

        return await Task<List<Tick>>.Factory.StartNew(
            () => {

              List<Tick> ret = new List<Tick>();

              using(var reader = new StreamReader(path))
              {
                //header
                reader.ReadLine();

                while (reader.Peek() >= 0)
                {
                  if (ct.IsCancellationRequested == true)
                  {
                    ct.ThrowIfCancellationRequested();
                  } 

                  Tick t;

                  var line = reader.ReadLine();
                  if (string.IsNullOrWhiteSpace(line)) continue;

                  var values = line.Split(',');
                  if (values.Count() < 4) continue;

                  t.Time = DateTime.ParseExact(values[0], "yyyy.MM.dd HH:mm", CultureInfo.InvariantCulture);
                  if (t.Time < startDate)
                    continue;
                  if (t.Time > endDate)
                    break;

                  t.Ask = Convert.ToDouble(values[1]);
                  t.Bid = Convert.ToDouble(values[2]);
                  t.Last = Convert.ToDouble(values[3]);
                  t.Volume = 0; //Convert.ToUInt32(values[3]);

                  ret.Add(t);
                }
              }

              return ret;
            },
          ct);
    }
  }
}
