using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BackTester
{
  public class DebugInfo
  {
    public string Info { get; set; }
    public int Severity { get; set; }

    public DebugInfo(string info, int severity) {

      Info = info;
      Severity = severity;
    }
  }
}
