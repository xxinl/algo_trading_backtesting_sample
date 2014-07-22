using System;
using System.Windows;

namespace BackTester.Views
{
  /// <summary>
  /// Interaction logic for MainWindow.xaml
  /// </summary>
  public partial class MainWindow : Window
  {
    public MainWindow()
    {
      InitializeComponent();
      
      this.Closed += (sender, args) => Application.Current.Shutdown();
    }
  }
}
