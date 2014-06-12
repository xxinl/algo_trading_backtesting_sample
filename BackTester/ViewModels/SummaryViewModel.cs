
using GalaSoft.MvvmLight.Messaging;
using OxyPlot;
using OxyPlot.Axes;
using System;
using System.Collections.Generic;
using OxyPlot.Series;
using DateTimeAxis = OxyPlot.Axes.DateTimeAxis;
using LinearAxis = OxyPlot.Axes.LinearAxis;
using LineSeries = OxyPlot.Series.LineSeries;

namespace BackTester.ViewModels
{
  public class SummaryViewModel : GalaSoft.MvvmLight.ViewModelBase
  {
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
    private PerformanceSummary _performanceSummary;

    public PlotModel PlotModelTick
    {
      get { return _plotModelTick; }
      set { _plotModelTick = value; RaisePropertyChanged(() => this.PlotModelTick); }
    }

    public PerformanceSummary PerformanceSummary
    {
      get { return _performanceSummary; }
      set
      {
        _performanceSummary = value;
        this.RaisePropertyChanged(()=>this.PerformanceSummary);
      }
    }

    public SummaryViewModel() {
      
      _setUpModel();

      Messenger.Default.Register<PerformanceTick>(this, _updateSeries);
      Messenger.Default.Register<PerformanceSummary>(this, (summary) => PerformanceSummary = summary);
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
                               StringFormat = "dd/MM/yy"
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

      var ss = new ScatterSeries {MarkerSize = 3, MarkerType = MarkerType.Circle };
      _plotModelTick.Series.Add(ss);
      ss = new ScatterSeries { MarkerSize = 3, MarkerType = MarkerType.Square };
      _plotModelTick.Series.Add(ss);
    }

    private void AddLineSeries(PlotModel model, int index, string title)
    {
      var ls = new LineSeries
      {
        StrokeThickness = 1,
        MarkerType = MarkerType.None,
        CanTrackerInterpolatePoints = false,
        Title = string.Format(title, index),
        Smooth = false,
      };
      
      model.Series.Add(ls);
    }

    private const int UPDATE_EVERY_X_HOUR = 4;
    private void _updateSeries(PerformanceTick t)
    {
      //only update graph each x hours
      if (!t.IsBalanceUpdated && 
        (t.Time.Minute != 0 || t.Time.Hour % UPDATE_EVERY_X_HOUR != 0)) return;

      _addPoint(PlotModelPer, 0, t.Time, t.Balance);
      _addPoint(PlotModelPer, 1, t.Time, t.Equity);
      _addPoint(PlotModelTick, 0, t.Time, t.Last);

      if (Math.Abs(t.Signal) == 1)
      {
        int index = t.Signal == 1 ? 1 : 2;
        var ss = PlotModelTick.Series[index] as ScatterSeries;
        if (ss != null)
        {
          var dp = new ScatterPoint(DateTimeAxis.ToDouble(t.Time), t.Last, 3, t.Signal);
          ss.Points.Add(dp);
        }
      }

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
