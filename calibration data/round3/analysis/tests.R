temp = round2_raw_ss
temp$temperature_sq = temp$temperature ^ 2
temp$temperature_cu = temp$temperature ^ 3
temp$humidity_sq = temp$humidity ^ 2
temp$humidity_cu = temp$humidity ^ 3
temp$custom_MICS_sq = temp$custom_MICS ^ 2
temp$custom_MICS_cu = temp$custom_MICS ^ 3

temp$NO2 = round2_ref_ss$NO2
fit_round2 <- lm(NO2 ~ temperature + temperature_sq + temperature_cu +
                   humidity + humidity_sq + humidity_cu +
                   custom_MICS + custom_MICS_sq + custom_MICS_cu, data=temp)
summary(fit_round2)

compared_round2 <- data.frame(timestamp = round2_ref_ss$timestamp, reference = round2_ref_ss$NO2, fitted = fitted(fit_round2))

print(paste('mean error', mean(compared_round2$reference - compared_round2$fitted), 'ppb'))
print(paste('mean absolute error', mean(abs(compared_round2$reference - compared_round2$fitted)), 'ppb'))
rm(temp)