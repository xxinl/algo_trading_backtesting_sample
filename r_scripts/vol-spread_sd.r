setwd("c:/workspace")
library(zoo)
library(lmtest)

file_pos <- 'eur2013_1.csv'

ret <- read.csv(file_pos, header = F)
ret <- read.zoo(ret, sep=",", header=F, 
                index.column=1, format="%Y-%b-%d %H:%M:%S", tz="",  
                aggregate = tail)

# plot(ret[,c(1,4)], screens = 1, col = 1:3)

# grangertest(V2 ~ V5, order = 1, data = ret)
# grangertest(V2 ~ V6, order = 1, data = ret)

ret$V5 = lag(ret$V4, 1)
ret$V6 = lag(ret$V4, -1)
# fit <- aov(V6 ~ V8, data=ret)

#--linear regression------------
fit <- lm(V5 ~ V2, data=ret)
summary(fit)
fit <- lm(V5 ~ V3, data=ret)
summary(fit)
fit <- lm(V5 ~ V4, data=ret)
summary(fit)
fit <- lm(V5 ~ V6, data=ret)
summary(fit)
fit <- lm(V5 ~ V4+V6, data=ret)
summary(fit)
fit <- lm(V5 ~ V2+V3+V4+V6, data=ret)
summary(fit)
# plot(ret$V8, ret$V5)
# abline(fit)

#---svm-------------
library(e1071)

