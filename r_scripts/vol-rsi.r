setwd("c:/workspace")
library(zoo)
library(lmtest)

file_pos <- 'eur_risk_rsi.csv'

ret <- read.csv(file_pos, header = F)
ret <- read.zoo(ret, sep=",", header=F, 
                index.column=1, format="%Y-%b-%d %H:%M:%S", tz="",  
                aggregate = tail)

ret$V5 = lag(ret$V2, 1)

#--linear regression------------
fit <- lm(V5 ~ V2, data=ret)
summary(fit)
fit <- lm(V5 ~ V3, data=ret)
summary(fit)
fit <- lm(V5 ~ V4, data=ret)
summary(fit)
fit <- lm(V5 ~ V2+V3+V4, data=ret)
summary(fit)
fit <- lm(V5 ~ V3+V4, data=ret)
summary(fit)
