using GalaSoft.MvvmLight.Messaging;
using System;

namespace BackTester.ViewModels
{
  public class DebugInfoViewModel : GalaSoft.MvvmLight.ViewModelBase
  {
    public bool IsDisplayDebug { get; set; }

    private string _message;
    public string Message {
      get { return _message; }
      set { _message = value; this.RaisePropertyChanged(()=>this.Message); }
    }

    public DebugInfoViewModel() {

      IsDisplayDebug = false;

      Messenger.Default.Register<DebugInfo>(this, (info) =>
      {
        //if (!IsDisplayDebug && info.Severity == 0) return;

        Message = info.Info + Environment.NewLine + Message;
      });
    }
  }
}
