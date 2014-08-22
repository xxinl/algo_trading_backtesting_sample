
using System.Windows;
using System.Windows.Media;
using BackTester.ViewModels;

namespace BackTester.Views
{
  /// <summary>
  /// Interaction logic for SummaryWin.xaml
  /// </summary>
  public partial class SummaryWin : Window
  {
    public SummaryWin()
    {
      InitializeComponent();

      CompositionTarget.Rendering += (sender, args) =>
                                     {
                                       Plot1.InvalidatePlot(true);
                                       Plot2.InvalidatePlot(true);
                                     };
    }

    public void Reset()
    {
      var dc = this.DataContext as SummaryViewModel;
      dc.Reset();
    }
  }
}
