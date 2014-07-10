setwd("c:/workspace")
library(zoo)
library(lmtest)

file_pos <- 'eur_day_opti_2013_15_1_1.csv'

ret <- read.csv(file_pos, header = F)
ret <- read.zoo(ret, sep=",", header=F, 
                index.column=1, format="%Y-%b-%d", tz="",  
                aggregate = sum)

# plot(ret[,c(1,4)], screens = 1, col = 1:3)

# grangertest(V2 ~ V5, order = 1, data = ret)
# grangertest(V2 ~ V6, order = 1, data = ret)

ret$V8 = lag(ret$V2, -1)
ret$V9 = lag(ret$V2, -2)
# fit <- aov(V6 ~ V8, data=ret)

fit <- lm(V5 ~ V8, data=ret)
summary(fit)
fit <- lm(V6 ~ V8, data=ret)
summary(fit)
# plot(ret$V8, ret$V5)
# abline(fit)
