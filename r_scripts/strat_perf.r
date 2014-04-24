setwd("c:/workspace")
library(zoo)
library(TTR)
library(PerformanceAnalytics)

file_pos <- 'o-h_ma_25-270_0.7.order'

ret <- read.csv(file_pos, header = F)
ret = subset(ret, V1==7 & V2==45)
ret$V10 = (ret$V8 - ret$V6)*ret$V9
ret = ret[,c(3,10)]
ret <- read.zoo(ret, sep=",", header=F, 
                index.column=1, format="%Y-%b-%d %H:%M:%S", tz="",  
                aggregate = sum)

charts.PerformanceSummary(ret, colorset=rich6equal)
table.Drawdowns(ret)
table.DownsideRisk(ret)
table.Stats(ret, ci = 0.95, digits = 4)
table.AnnualizedReturns(ret, scale = NA, Rf = 0,
                        geometric = TRUE, digits = 4)
