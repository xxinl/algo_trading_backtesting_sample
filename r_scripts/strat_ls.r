setwd("c:/workspace")
library(zoo)
library(TTR)

fileEvents <- '2013.csv'
fileRates <- 'eurusd_min_2013.csv'

calcRtn <- function(r)
  if(as.numeric(r['r_obser']) > as.numeric(r['rates'])) {
    if(as.numeric(r['r_max']) > as.numeric(r['r_obser']) + as.numeric(r['r_sd'])){
      0 - as.numeric(r['r_sd'])
    } else{
      as.numeric(r['r_obser']) - as.numeric(r['r_hold'])
    }
  } else{
    if(as.numeric(r['r_min']) < as.numeric(r['r_obser']) - as.numeric(r['r_sd'])){
      0 - as.numeric(r['r_sd'])
    }else{
      as.numeric(r['r_hold']) - as.numeric(r['r_obser'])
    }
  }

# p_hold_win = 60
# p_obser_win = 5
# p_sd_lookback = 7200 #5 days
# p_sd_divisor = 1

funcGetRtn = function(rates, events, p_obser_win = 5, p_hold_win = 15, 
						p_sd_lookback = 7200, p_sd_divisor = 1){
	r_sd <- runSD(diff(rates, p_obser_win), p_sd_lookback)
#   r_sd <- rollapply(diff(rates, p_obser_win), p_sd_lookback, sd, align = 'right')

	r_obser <- lag(rates, p_obser_win)
	r_hold <- lag(rates, p_obser_win + p_hold_win)
  r_min <- rollapply(rates, list(seq(p_obser_win, p_hold_win)), min)
	r_max <- rollapply(rates, list(seq(p_obser_win, p_hold_win)), max)

	fin <- na.omit(merge(rates, events, r_sd, r_obser, r_hold, r_min, r_max))[,c(1,4:8)]

	pos <-  subset(fin, abs(as.numeric(fin$r_obser) - as.numeric(fin$rates)) > as.numeric(fin$r_sd)/p_sd_divisor)

# 	ret <- sign(as.numeric(pos$r_obser) - as.numeric(pos$rates)) * (as.numeric(pos$r_obser) - as.numeric(pos$r_hold))

  ret <- rollapply(pos, 1, calcRtn, by.column = FALSE)

# 	plot(1:NROW(ret), cumsum(ret), type = 'l')

#   x <- ret
#   h<-hist(x, breaks=50, col="red", xlab="P/L", 
#           main="H N") 
#   xfit<-seq(min(x),max(x),length=50) 
#   yfit<-dnorm(xfit,mean=mean(x),sd=sd(x)) 
#   yfit <- yfit*diff(h$mids[1:2])*length(x) 
#   lines(xfit, yfit, col="blue", lwd=2)
	
	return(c(
		ttl_rtn = sum(ret), 
		mean_rtn = mean(ret),
		sd_rtn = sd(ret),
		sharpe = mean(ret)/sd(ret), 
		critical_v = mean(ret)/sd(ret)*sqrt(length(ret)), 
		no_pos = NROW(pos),
		win_loss_ratio = NROW(ret[ret>0])/NROW(ret[ret<0])))
}

events <- read.csv(fileEvents, header = F)
events <- subset(events[,c(1,2,4,6)], 
                 (V4=='usd' | V4 == 'eur') & (V6=='Medium' | V6=='High')
                 & V2 != '')
events <- read.zoo(events, sep=",", header=F, 
                   index.column=1:2, format="%d-%b-%y %H:%M", tz="",  
                   aggregate = function(x) tail(x, 1))

rates <- read.zoo(fileRates, sep=",", header=TRUE, 
                  index.column=1:2, format="%Y%m%d %H%M%S", tz="", 
                  colClasses = rep(c("NULL", "character", "numeric"), c(1, 2, 5)))
rates <- rates[,c(4)]

# funcGetRtn(rates, events, 5, 15, 7200, 1)

#########################################

library(foreach)
library(doParallel)
# cl <- makeCluster(2)
registerDoParallel()

results <- foreach (obser_win = 1:30, .combine=rbind) %dopar% {
  library(zoo)
  library(TTR)
  
	if( 5 > obser_win)
		hold_win_start = 5
	else
		hold_win_start = obser_win + 1

	for (hold_win in hold_win_start:120) {  
		rtn <- funcGetRtn(rates, events, obser_win, hold_win, 7200, 1)
		tmp <- c(obser = obser_win, hold = hold_win, 
					rtn[1], rtn[2], rtn[3], rtn[4], 
					rtn[5], rtn[6], rtn[7])
# 		print(tmp)	
    tmp
	}
}

##########################################

# library("optimx")
# 
# min_fun <- function(rates, events, par){
#   0 - funcGetRtn(rates, events, par[1], par[2], par[3], par[4])
# }
# 
# opti_result <- optim(par = c(5, 15, 7200, 3), min_fun, rates = rates, events = events)
