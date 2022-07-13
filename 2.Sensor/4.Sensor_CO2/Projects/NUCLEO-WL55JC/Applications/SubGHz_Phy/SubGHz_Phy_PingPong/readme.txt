+Sensor SHT30:
Pin: ok - (4.2v = 100%, 3.2v = 1%)
|----Name----|-----status----|-------Time(s)-----|--------energy(uA)------|
Lowpower 			ok                60.0                  8.90
Join Network 		ok                6.75                  8.42
Send Meseeger 		ok                9.00                  7.68
Delete Node 		ok                9.00                  8.94
Set time wakeup 	ok                9.00                  7.56



/* Private functions ---------------------------------------------------------*/
#if APP_TASK_TEST
static void JoinNetworkTask_Process(void)
{
  uint8_t i = 0;
  if (VarApp.FlagStartReadSensor == 0)
  {
    if (VarApp.CountStartReadSensor >= 2)
    {
      SmartFram_SYS_ReadAllSensor();
			VarApp.CountTimeOut++;
      if ((VarApp.CountStopReadCheckSensor >= 15)||(VarApp.CountTimeOut >= 75)||(VarApp.CountStopReadCheckSensor <=4 && VarApp.CountTimeOut >= 10))
      {
				Sys_DataSensorRead.Data_Senser_CO2 = 0;
				APP_PPRINTF("\n---------COUN: %d-------------\n", VarApp.CountStopReadCheckSensor);
				for (i = 5; i < 15; i++)
        {
           Sys_DataSensorRead.Data_Senser_CO2 += VarApp.ArrData_CO2[i];
        }
				Sys_DataSensorRead.Data_Senser_CO2 /= 10;
        APP_PPRINTF("\n---------%d-----%d--------\n", Sys_DataSensorRead.Data_Senser_CO2, Sys_DeviceFlashData_MSG.CO2);
				
				if((vr_Error <= 5)&&(Sys_DataSensorRead.Data_Senser_CO2 == 0))
				{
					vr_Error += 1;
					Sys_DataSensorRead.Data_Senser_CO2 = Sys_DeviceFlashData_MSG.CO2;
				}
				else
				{
					vr_Error = 0;
				}
		
        VarApp.FlagStartReadSensor = 1;
        //FS_StartSendMsg();
				smartfram_IncrementIdMsg(Sys_DataSensorRead.Data_Senser_CO2);
        smartfram_CO2_LowpowerWakeupMode(0);
  
      }
      else
      {
        if ( Sys_DataSensorRead.Data_Senser_CO2 == 410)
        {
					if (VarApp.CountStopReadCheckSensor < 1)
          {
          VarApp.ArrData_CO2[VarApp.CountStopReadCheckSensor] = Sys_DataSensorRead.Data_Senser_CO2;
          VarApp.CountStopReadCheckSensor ++;
					}
        }
        else if ( Sys_DataSensorRead.Data_Senser_CO2 == 511)
        {
					if (VarApp.CountStopReadCheckSensor <3)
          {
						VarApp.ArrData_CO2[VarApp.CountStopReadCheckSensor] = Sys_DataSensorRead.Data_Senser_CO2;
						VarApp.CountStopReadCheckSensor ++;
					}
        }
        else if ( Sys_DataSensorRead.Data_Senser_CO2 == 500)
        {
          if (VarApp.CountStopReadCheckSensor < 5)
          {
            VarApp.ArrData_CO2[VarApp.CountStopReadCheckSensor] = Sys_DataSensorRead.Data_Senser_CO2;
            VarApp.CountStopReadCheckSensor ++;
          }
        }
        else if (Sys_DataSensorRead.Data_Senser_CO2 > 500)
        {
          VarApp.ArrData_CO2[VarApp.CountStopReadCheckSensor] = Sys_DataSensorRead.Data_Senser_CO2;
          VarApp.CountStopReadCheckSensor ++;
        }
				
        for (i = 0; i < 20; i++)
        {
          APP_PPRINTF("%d ", VarApp.ArrData_CO2[i]);
        }
        APP_PPRINTF("\n---------%d-----%d----%d----\n", VarApp.CountStopReadCheckSensor, Sys_DataSensorRead.Data_Senser_CO2, VarApp.CountTimeOut);
				
      }
    }
    else
    {
      VarApp.CountStartReadSensor++;
    }

  }

  //SmartFram_SYS_ReadAllSensor();
  //Smartfram_SYS_DebugSensor(1);

}
#endif
