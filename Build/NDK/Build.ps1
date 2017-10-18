#This Build.ps1 is designed for building apk's.
$Mode="Release";
$Debug = $False;
$Clean = $False;
$Install = $False;
$Uninstall = $False;
$Execute = $False;
$Gdb = $False;
$Kill = $False;
$PlatformLevel = 0;
$TargetSDKLevel = 0;
$MinSDKLevel = 0;
$DeviceTarget = 0;
$DeviceName = "";
for($i=0;$i -lt $args.length; $i++){
   if($args[$i].CompareTo("Debug") -eq 0){
      $Debug=$True;
      $Mode="Debug";
   }Elseif($args[$i].CompareTo("Clean") -eq 0){
      $Clean=$True;
   }Elseif($args[$i].CompareTo("Install") -eq 0){
      $Install = $True;
   }Elseif($args[$i].CompareTo("Uninstall") -eq 0){
      $UnInstall = $True;
   }Elseif($args[$i].CompareTo("Execute") -eq 0){
      $Execute = $True;
   }Elseif($args[$i].CompareTo("Kill") -eq 0){
	  $Kill = $True;
   }Elseif($args[$i].CompareTo("gdb") -eq 0){
	  $Gdb = $True;
   }Elseif($args[$i].CompareTo("PlatformLevel") -eq 0){
		$PlatformLevel = $args[$i+1];
		$i++;
   }Elseif($args[$i].CompareTo("SDKLevel") -eq 0){
		$MinSDKLevel = $args[$i+1];
		$TargetSDKLevel = $args[$i+1];
		$i++;
   }Elseif($args[$i].CompareTo("MinSDKLevel") -eq 0){
		$MinSDKLevel = $args[$i+1];
		$i++;
   }Elseif($args[$i].CompareTo("TargetSDKLevel") -eq 0){
		$TargetSDKLevel = $args[$i+1];
		$i++;
   }Elseif($args[$i].CompareTo("Device") -eq 0){
	 $DeviceTarget = $args[$i+1];
	 $i++;
   }
}

if((Test-Path env:\ANDROID_NDK_ROOT) -eq $FALSE){
  Write-Host "Please set the ANDROID_NDK_ROOT environment variable to the ndk directory."
  Exit
}
if((Test-Path env:\ANDROID_HOME) -eq $FALSE){
   Write-Host "Please set the ANDROID_HOME environment variable to the android sdk directory."
   Exit
}
if((Test-Path env:\ANT_HOME) -eq $FALSE){
  Write-Host "Please set the ANT_HOME environment variable to the ant directory."
  Exit
}
if((Test-Path env:\JAVA_HOME) -eq $FALSE){
   Write-Host "Please set the JAVA_HOME environment variable to the java development kit home directory."
   Exit
}
if($Install -Or $Execute -Or $Kill -Or $Gdb -Or $Uninstall -Or $Minimize){
	$DeviceOutput = [string](& "$($Env:ANDROID_HOME)\platform-tools\adb.exe" devices)
	$DeviceLines = $DeviceOutput.Split(' ');
	if($DeviceTarget -ge ($DeviceLines.length-5)){
		Write-Host "Could not find the specefied device by index(or no device is connected.)"
		Exit
	}
	$DeviceName = $DeviceLines[$DeviceTarget+4].Split('')[0];
}
Write-Host "Building in mode: $($Mode) PlatformLevel: $($PlatformLevel) MinSDK: $($MinSDKLevel) TargetSDK: $($TargetSDKLevel) $($DeviceName)"
$Progs = @()
$Args = @()
if($Clean){
   $Progs = $Progs+"$($Env:ANDROID_NDK_ROOT)\ndk-build.cmd";
   $Progs = $Progs+"$($Env:ANT_HOME)\bin\ant.bat";
   $Args = $Args+"clean";
   $Args = $Args+"clean";
   if($Debug -eq $true){
      $Args[0] = "NDK_DEBUG=1 clean";
      $Args[1] = "debug clean";
   }
}elseif($Install){
   $Files = Get-ChildItem bin\*.apk
   $TargetFile = "";
   for($i = 0; $i -lt $Files.length; $i++){
      if($Debug -eq $True){
         if($Files[$i].name -like "*debug*" -And $Files[$i].name -NotLike "*unaligned*"){
            $TargetFile = $Files[$i].name;
            break;
         }
      }else{
         if($Files[$i].name -NotLike "*debug*" -And $Files[$i].name -NotLike "*unaligned*"){
            $TargetFile = $Files[$i].name;
            break;
         }
      }
   }
   Write-Host "Target: $($TargetFile)"
   if($TargetFile -like "*.apk"){
      Write-Host "Discovered and installing: $($TargetFile)"
      $Progs = $Progs+"$($Env:ANDROID_HOME)\platform-tools\adb.exe";
      $Args = $Args+"-s $($DeviceName) install -r bin\$($TargetFile)";
   }else{
      Write-Host "Error: please build for the mode you intend to install for."
      Exit
   }
}elseif($Execute -Or $Uninstall -Or $Minimize -Or $Kill){
   $TargetFile = Get-ChildItem AndroidManifest.xml
   if($TargetFile.length -eq 0){
      Write-Host "No AndroidManifest.xml detected!"
      Exit
   }
   Write-Host "Parsing: $($TargetFile.name)"
   [xml]$TXML = Get-Content $TargetFile
   Write-Host "Discovered Package: $($TXML.manifest.package)";
   Write-Host "Discovered Activity: $($TXML.manifest.application.activity.name)";
   $Progs = $Progs+"$($Env:ANDROID_HOME)\platform-tools\adb.exe";
   if($Execute){
      $Args = $Args+"-s $($DeviceName) shell am start -n $($TXML.manifest.package)/$($TXML.manifest.application.activity.name)";
   }elseif($Kill){
	  $Args = $Args+"-s $($DeviceName) shell am force-stop $($TXML.manifest.package)";
   }else{
      $Args = $Args+"-s $($DeviceName) uninstall $($TXML.manifest.package)"
   }
}else{

	if($PlatformLevel -ne 0){
		Write-Host "Updating application.mk for new platform level."
		$ReplaceStr = "APP_PLATFORM := android-$($PlatformLevel)"
		$FileContents = (Get-Content ./jni/Application.mk)
		$FileContents = $FileContents -replace 'APP_PLATFORM := android-.+',$ReplaceStr
		Set-Content -Path ./jni/Application.mk -Value $FileContents
	}
	if($MinSDKLevel -ne 0){
		Write-Host "Updating AndroidManifest.xml for MinSDK level."
		$ReplaceStr = 'minSdkVersion="'+$MinSDKLevel+'"';
		$FileContents = (Get-Content AndroidManifest.xml);
		$FileContents = $FileContents -replace 'minSdkVersion="[0-9]+"',$ReplaceStr
		Set-Content -Path AndroidManifest.xml -Value $FileContents	
	}
	if($TargetSDKLevel -ne 0){
		Write-Host "Updating AndroidManifest.xml and project.properties for TargetSDK level."
		$ReplaceStrA = 'targetSdkVersion="'+$TargetSDKLevel+'"';
		$ReplaceStrB = "target=android-$($TargetSDKLevel)"
		$FileContents = (Get-Content AndroidManifest.xml);
		$FileContents = $FileContents -replace 'targetSdkVersion="[0-9]+"',$ReplaceStrA
		Set-Content -Path AndroidManifest.xml -Value $FileContents
		$FileContents = (Get-Content project.properties);
		$FileContents = $FileContents -replace 'target=android-.+', $ReplaceStrB
		Set-Content -Path project.properties -Value $FileContents
	}
   $Progs = $Progs+"$($Env:ANDROID_NDK_ROOT)\ndk-build.cmd";
   $Progs = $Progs+"$($Env:ANT_HOME)\bin\ant.bat";
   $Args = $Args+"";
   $Args = $Args+"release";
   if($Debug -eq $true){
      $Args[0] = "NDK_DEBUG=1";
      $Args[1] = "debug";
   }
}

$DataHandler = {
	Write-Host $Event.SourceEventArgs.Data;
}

$pinfo = New-Object System.Diagnostics.ProcessStartInfo
$pinfo.RedirectStandardError = $true
$pinfo.RedirectStandardOutput = $true
$pinfo.UseShellExecute = $false
for($i = 0; $i -lt $Progs.length; $i++){
   $p = New-Object System.Diagnostics.Process
   $pinfo.FileName = $Progs[$i];
   $pinfo.Arguments = $Args[$i];
   $p.StartInfo = $pinfo;
   
   Register-ObjectEvent -inputObject $p -EventName "OutputDataReceived" -Action $DataHandler -SupportEvent;
   Register-ObjectEvent -inputObject $p -EventName "ErrorDataReceived" -Action $DataHandler -SupportEvent;

   $p.Start();
   $p.BeginOutputReadLine();
   $p.BeginErrorReadLine();
   while($p.HasExited -eq $false){}
   if($p.ExitCode -ne 0){
      break;
   }
}
