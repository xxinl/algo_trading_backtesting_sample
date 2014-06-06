using System.Windows.Media;
using GalaSoft.MvvmLight.Messaging;
using OxyPlot;
using OxyPlot.Axes;
using System;
using System.Collections.Generic;
using System.Linq;
using DateTimeAxis = OxyPlot.Axes.DateTimeAxis;
using LinearAxis = OxyPlot.Axes.LinearAxis;
using LineSeries = OxyPlot.Series.LineSeries;

namespace BackTester.ViewModels
{
  public class SummaryViewModel : GalaSoft.MvvmLight.ViewModelBase
  {
    private readonly List<OxyColor> _colors = new List<OxyColor>
                                            {
                                                OxyColors.Green,
                                                OxyColors.IndianRed,
                                                OxyColors.Coral,
                                                OxyColors.Chartreuse,
                                                OxyColors.Azure
                                            };

    private PlotModel _plotModelPer;
    public PlotModel PlotModelPer
    {
      get { return _plotModelPer; }
      set
      {
        _plotModelPer = value; 
        RaisePropertyChanged(() => this.PlotModelPer);
      }
    }

    private PlotModel _plotModelTick;
    public PlotModel PlotModelTick
    {
      get { return _plotModelTick; }
      set { _plotModelTick = value; RaisePropertyChanged(() => this.PlotModelTick); }
    }
    

    public SummaryViewModel() {
      
      _setUpModel();

      Messenger.Default.Register<PerformanceTick>(this, _updateSeries);
    }

    private void _setUpModel()
    {
      _plotModelPer = new PlotModel
                      {
                        LegendOrientation = LegendOrientation.Horizontal,
                        LegendPlacement = LegendPlacement.Outside,
                        LegendPosition = LegendPosition.TopRight,
                        LegendBackground = OxyColor.FromAColor(200, OxyColors.White),
                        LegendBorder = OxyColors.Black
                      };

      _plotModelPer.Axes.Add(new DateTimeAxis()
                             {
                               MajorGridlineStyle = LineStyle.Solid,
                               MinorGridlineStyle = LineStyle.Dot,
                               IntervalLength = 30,
                               Position = AxisPosition.Bottom,
                               StringFormat = "MM/dd"
                             });

      _plotModelPer.Axes.Add(new LinearAxis()
                             {
                               MajorGridlineStyle = LineStyle.Solid,
                               MinorGridlineStyle = LineStyle.Dot,
                               Position = AxisPosition.Left
                             });

      _plotModelTick = new PlotModel
                       {
                         LegendOrientation = LegendOrientation.Horizontal,
                         LegendPlacement = LegendPlacement.Outside,
                         LegendPosition = LegendPosition.TopRight,
                         LegendBackground = OxyColor.FromAColor(200, OxyColors.White),
                         LegendBorder = OxyColors.Black
                       };

      _plotModelTick.Axes.Add(new DateTimeAxis()
                             {
                               MajorGridlineStyle = LineStyle.Solid,
                               MinorGridlineStyle = LineStyle.Dot,
                               IntervalLength = 30,
                               Position = AxisPosition.Bottom,
                               StringFormat = "MM/dd"
                             });

      _plotModelTick.Axes.Add(new LinearAxis()
                             {
                               MajorGridlineStyle = LineStyle.Solid,
                               MinorGridlineStyle = LineStyle.Dot,
                               Position = AxisPosition.Left
                             });

      AddLineSeries(_plotModelPer, 0, "bal");
      AddLineSeries(_plotModelPer, 1, "eq");
      AddLineSeries(_plotModelTick, 0, "t");
    }

    private void AddLineSeries(PlotModel model, int index, string title)
    {
      var ls = new LineSeries
      {
        StrokeThickness = 2,
        MarkerType = MarkerType.None,
        CanTrackerInterpolatePoints = false,
        Title = string.Format(title, index),
        Smooth = false,
      };
      
      model.Series.Add(ls);
    }

    private void _updateSeries(PerformanceTick t)
    {
      //only update graph each 4 hours
      if (t.Time.Minute != 0 || t.Time.Hour % 4 != 0) return;

      _addPoint(PlotModelPer, 0, t.Time, t.Balance);
      _addPoint(PlotModelPer, 1, t.Time, t.Equity);
      _addPoint(PlotModelTick, 0, t.Time, t.Last);
    }

    private void _addPoint(PlotModel model, int index, DateTime time, double value)
    {
      var ls = model.Series[index] as LineSeries;
      if (ls != null)
      {
        var dp = new DataPoint(DateTimeAxis.ToDouble(time), value);
        ls.Points.Add(dp);
      }
    }
  }
}
