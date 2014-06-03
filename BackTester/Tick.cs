using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BackTester
{
  public struct Tick
  {
    public DateTime Time;

    public double Ask;

    public double Bid;

    public double Last;

    public uint Volume;
  }
}
