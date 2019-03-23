@echo off

if %username% == php (
	cd Release
	upx.exe mss32.dll
	cp mss32.dll "C:\Program Files (x86)\Steam\steamapps\common\Call of Duty 2\mss32.dll"
)

if %username% == kung (
	cd Release
	copy mss32.dll C:\games\cod2_1_3_clean_for_mapping
)
