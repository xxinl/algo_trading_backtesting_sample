using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Practices.ServiceLocation;
using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Ioc;

namespace BackTester
{
  public class ViewModelLocator
  {
    static ViewModelLocator() {

      ServiceLocator.SetLocatorProvider(() => SimpleIoc.Default);
      SimpleIoc.Default.Register<BackTesterViewModel>();
      SimpleIoc.Default.Register<DebugInfoViewModel>();
    }

    public BackTesterViewModel BackTester {
      get {
        return ServiceLocator.Current.GetInstance<BackTesterViewModel>();
      }
    }

    public DebugInfoViewModel MessageWin
    {
      get
      {
        return ServiceLocator.Current.GetInstance<DebugInfoViewModel>();
      }
    }
  }
}
