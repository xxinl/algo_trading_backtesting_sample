
using System;
using Microsoft.Practices.ServiceLocation;
using GalaSoft.MvvmLight.Ioc;
using BackTester.ViewModels;

namespace BackTester
{
  public class ViewModelLocator
  {
    static ViewModelLocator() {

      ServiceLocator.SetLocatorProvider(() => SimpleIoc.Default);
      SimpleIoc.Default.Register<BackTesterViewModel>();
      SimpleIoc.Default.Register<DebugInfoViewModel>();
      SimpleIoc.Default.Register<SummaryViewModel>();
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

    public SummaryViewModel SummaryWin
    {
      get
      {
        return ServiceLocator.Current.GetInstance<SummaryViewModel>(Guid.NewGuid().ToString());
      }
    }
  }
}
