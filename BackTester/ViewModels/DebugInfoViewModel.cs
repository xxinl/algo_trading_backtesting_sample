using GalaSoft.MvvmLight.Messaging;
using System;

namespace BackTester.ViewModels
{
  public class DebugInfoViewModel : GalaSoft.MvvmLight.ViewModelBase
  {
    private string _message;
    public string Message {
      get { return _message; }
      set { _message = value; this.RaisePropertyChanged(()=>this.Message); }
    }

    public DebugInfoViewModel() {


      Messenger.Default.Register<DebugInfo>(this, (info) =>
      {
        //if (!IsDisplayDebug && info.Severity == 0) return;

        Message = info.Info + Environment.NewLine + Message;
      });
    }
  }
}
