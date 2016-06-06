#
# GenericBuildScript.pl -- Nightly Build script that uses a configuration file to guide installations
#
# This script supports the following
#     - checkout a clean Perforce source tree into a new directory
#     - Build projects via the VC 8-2015 command line (includes documentation)
#     - email the build errors (if any) to the necessary recipients
#
# $File: GenericBuildScript.pl $
# $Revision: #1 $
# $Author: plohrman $
#
# Copyright (c) 2006-2016 , Advanced Micro Devices, Inc.  All rights reserved.

#--- External Packages ---

use strict;
use warnings;

use File::Basename;
use File::Copy;
use File::Glob 'bsd_glob';
use File::Path;
use File::Find;
use Getopt::Long;
use Mail::Sender;
use Cwd;
use Win32;
use Class::Struct;
use LWP::Simple::Post qw (post post_xml);
use LWP::Simple;
use Archive::Zip qw(:ERROR_CODES :CONSTANTS);
use POSIX qw(strftime);
use Archive::Extract;

## enable autoflush on STDOUT
$| = 1;

my $OptConfig;           # default undef
my $OptBuild;            # default undef
my $OptClean;            # default undef
my $OptDebug;            # default undef
my $OptRelease;          # default undef
my $OptReleaseInternal;  # default undef
my $OptReleaseNda;  	 # default undef
my $OptImage;            # default undef
my $OptInstall;          # default undef
my $OptHelp;             # default undef
my $OptLabel;            # default undef

my $OptMajorVersion;     # default undef
my $OptMinorVersion;     # default undef
my $OptBuildVersion;     # default undef
my $OptUpdateVersion;    # default undef
my $OptVersion;          # default undef
my $OptMail;             # default undef
my $OptVerbose;          # default undef
my $OptDocumentation;    # default undef
my $OptInternal;         # default undef
my $OptPredefine;        # default undef
my $OptLog;              # default undef
my $OptTest;             # default undef

## TODO: replace this to search for the required resources within the cwd
my $gHomeDirectory = getcwd();
$gHomeDirectory =~ s/\//\\/g;
if (not ( $gHomeDirectory =~ /.*\\Installer$/ ) )
{
   Error( "The working directory of $gHomeDirectory will not allow the build script to work properly." );
}

my $gInstallFailed = 0;
my $gBuildFailed = 0;
my $gBuildFailedMessage = "";

my $gEmailServer = 'aussmtp.amd.com';
my $gEmailSender = 'dl.DevToolsBOS@amd.com';

my $gJenkinsURL = 'http://bdclin64-gdt-jenkins.amd.com:8080';
my $gBuildStartTime = time;

my $gVersion = "[MAJOR].[MINOR].[BUILD].[UPDATE]";

my %gVariables;

## These are set by the PerformSync subroutine
my %gP4Settings;

## This is used by the PrependTextToFile File::Find callback
my $gPrependTextFileFilter = "";
my $gPrependTextFile = "";

my $gTestcaseTimeout = 600;

## This is used for testing;
my @gTestResultDIR;
my $gResultRoot = "";
my $alreadyRefreshedDTKDepot = "false"; 

struct P4Setting => {
   root     => '$',  # string path to the root of the tree
   client   => '$',  # string name of the clientspec to use
   server   => '$',  # string the name of the perforce server
   user     => '$',  # string the user to access the server as
};

# Constant Definitions
use constant LOG => 1;   # use to indicate message to be written to logfile in Vprint and MySystem
use constant TRUE => 1;
use constant FALSE => 0;

# Usage Text - used for error and help displays
my $UsageText =
    "\n\n" .
    'Generic Build script $Revision: #1 $' .
    "\n\nUsage: $0 [options...]\n\n" .
    "The supported options are:\n\n" .
    "\t--config file.txt\t\tthe configuration file which guides the build\n" .
    "\t--build\t\t\t\texecutes the build portion of the configuration file\n" .
    "\t--BuildandTest\t\t\t\texecutes the build portion of the configuration file but exit on errors\n" .
    "\t--clean\t\t\t\texecutes the clean portion of the configuration file\n" .
    "\t--debug\t\t\t\texecutes the debug portion of the configuration file\n" .
    "\t--release\t\t\t\texecutes the release portion of the configuration file\n" .
    "\t--releaseinternal\t\t\t\texecutes the ReleaseInternal portion of the configuration file\n" .
    "\t--releasenda\t\t\t\texecutes the ReleaseNda portion of the configuration file\n" .
    "\t--image\t\t\t\texecutes the image portion of the configuration file\n" .
    "\t--install\t\t\texecutes the install portion of the configuration file\n" .
    "\t--test\t\t\t\texecutes the test portion of the configuration file\n" .
    "\t--internal\t\t\tallows control over steps to perform on internal builds\n" .
    "\t--label\t\t\t\tAll sync's will be done to the specified label\n" .
#    "\t--documentation\t\t\tcopy documentation to web server\n" .
    "\t--help\t\t\t\tshow this help text\n" .
    "\t--mail user\@somewhere.com \temail build results\n" .
    "\t--verbose\t\t\tdisplay detailed information\n" .
    "\t--version\t\t\tincrement build version number\n" .
    "\t--majorversion\t\t\tincrement major version number\n" .
    "\t--minorversion\t\t\tincrement minor version number\n" .
    "\t--buildversion\t\t\tincrement build version number\n" .
    "\t--updateversion\t\t\tincrement update version number\n" .
    "\t--predefine\t\t\tpredefine variables used within the configuration file\n" .
    "\t--log\t\t\t\tLog file to append output to\n";


# Parse command line options
GetOptions (
            'verbose'       => \$OptVerbose,
            'config=s'      => \$OptConfig,
            'build'         => \$OptBuild,
            'clean'         => \$OptClean,
            'debug'         => \$OptDebug,
            'release'       => \$OptRelease,
            'releaseinternal' => \$OptReleaseInternal,
			'releasenda'	=> \$OptReleaseNda,
            'image'         => \$OptImage,
            'label=s'       => \$OptLabel,
            'install'       => \$OptInstall,
            'internal'      => \$OptInternal,
            'test'          => \$OptTest,
            'documentation' => \$OptDocumentation,
            'mail=s'        => \$OptMail,
            'predefine=s'   => \$OptPredefine,
            'version'       => \$OptVersion,
            'majorversion'  => \$OptMajorVersion,
            'minorversion'  => \$OptMinorVersion,
            'buildversion'  => \$OptBuildVersion,
            'updateversion' => \$OptUpdateVersion,
            'help'          => \$OptHelp,
            'log=s'         => \$OptLog );

## Make a hash of the possible states and whether they should be performed:

my %states = (
   'build'        => (defined $OptBuild),
   'clean'        => (defined $OptClean),
   'debug'        => (defined $OptDebug),
   'release'      => (defined $OptRelease),
   'releaseinternal' => (defined $OptReleaseInternal),
   'releasenda'   => (defined $OptReleaseNda),
   'image'        => (defined $OptImage),
   'install'      => (defined $OptInstall),
   'internal'     => (defined $OptInternal),
   'version'      => (defined $OptVersion),
   'test'         => (defined $OptTest),
   'predefine'    => (defined $OptPredefine),
   'majorversion' => (defined $OptMajorVersion),
   'minorversion' => (defined $OptMinorVersion),
   'notbuild'        => (not defined $OptBuild),
   'notclean'        => (not defined $OptClean),
   'notdebug'        => (not defined $OptDebug),
   'notrelease'      => (not defined $OptRelease),
   'notimage'        => (not defined $OptImage),
   'notinstall'      => (not defined $OptInstall),
   'notinternal'     => (not defined $OptInternal),
   'notversion'      => (not defined $OptVersion),
   'nottest'         => (not defined $OptTest),
   'notpredefine'    => (not defined $OptPredefine),
   'notmajorversion' => (not defined $OptMajorVersion),
   'notminorversion' => (not defined $OptMinorVersion),
   'notbuildversion' => (not defined $OptBuildVersion),
   'notupdateversion' => (not defined $OptUpdateVersion),
   'always'          => 1,
);

for my $key ( keys %states )
{
   my $value = $states{ $key };
   Vprint( $OptVerbose, "KEY: $key \tVALUE: $value\n" );
}

## Initialize the log
my $gLogFile;           ## This can be updated within the script using the Log keyword
if ( defined $OptLog and -e $OptLog )
{
   ## the user specified a log to build on at the command line
   $gLogFile = $OptLog;
   Vprint( LOG, "=====================================================================================================" );
   Vprint( LOG, "Continuing logfile in new script" );
   Vprint( LOG, "=====================================================================================================" );
}
elsif ( defined $OptLog )
{
   $gLogFile = $OptLog;
   Vprint( LOG, "=====================================================================================================" );
   Vprint( LOG, "Starting logfile as specified by command line" );
   Vprint( LOG, "=====================================================================================================" );
}

# Generate default output filename based on current time & date
my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();

my $gDate = sprintf "%04s-%02s-%02s", $year+1900, $mon+1, $mday;
my $gDate2 = sprintf "%02s%02s.%02s%02s", $year-100, $mon+1, $mday, $hour;
my $gTime = sprintf "%02s%02s", $hour, $min;

my @monthString = qw( January February March April May June July August September October November December );
my @dayString = qw( 1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th 16th 17th 18th 19th 20th 21st 22nd 23rd 24th 25th 26th 27th 28th 29th 30th 31st );

AddVariable( "DAY",  $mday );
AddVariable( "DAY2", $dayString[$mday] );
AddVariable( "MONTH", $monthString[$mon] );
AddVariable( "MONTHNUMBER", $mon+1 );
AddVariable( "YEAR", $year+1900 );
AddVariable( "DATE", $gDate );
AddVariable( "DATE2", $gDate2 );
AddVariable( "TIME", $gTime );
AddVariable( "DTKDIR", "$gHomeDirectory/../Dtk/InstallRoot" );
AddVariable( "DTKIP", "127.0.0.1" );

## add the AUTOBUILDDIR variable if it exists as an environment variable
## this can be used to set a build to a particular drive for a machine.
if ( exists $ENV{'AutoBuildDir'} )
{
   my $dir = $ENV{'AutoBuildDir'};
   Vprint( LOG, "Setting build directory to '$dir' based on environment settings." );
   AddVariable( "AUTOBUILDDIR", $dir );
}

if ( defined $OptMail )
{
   AddVariable( "EMAIL", $OptMail );
}

## define predefined variables
if ( defined $OptPredefine )
{
   my @predefines = split( /\;/, $OptPredefine );

   foreach( @predefines )
   {
      my ( $variable, $value ) = split( /=/, $_ );

      if ( defined $variable and
           defined $value )
      {
         AddVariable( "\U$variable", $value );
      }
   }
}


PrintHelpAndExit() if (defined $OptHelp or not defined $OptConfig);

## $OptConfig contains the path to the configuration file for this build

(open INFILE, $OptConfig) or
   Error( "Cannot open file: $OptConfig for reading: $!" );

chomp (my @lines = <INFILE>);
close INFILE;

@lines = PreProcessConfiguration( @lines );

my $gState = "";
my $gbStateLine = "false";
my $bProcessLine = "false";
my $currLineNumber = 0;
foreach( @lines )
{
   my $origLine = $_;
   $currLineNumber++;

   if ( $origLine =~ /^\s*$/ )
   {
      next;
   }

   my $line = ProcessForVariables( $origLine );

   ## check the state
   ## these states correspond to command line options
   ## if the option wasn't set, then the corresponding states will be ignored

   ## first check if the line contains only states, ifdef, ifndef, ifdefeq, ifdefneq
   my @words = split( / /, "\L$line" );

   my $bContainsOnlyStates = "true";
   my $nSkipThisMany = 0;
   foreach ( @words )
   {
      if ( $nSkipThisMany > 0 )
      {
         $nSkipThisMany -= 1;
         next;
      }
      elsif ( $_ =~ /^ifdef$/i or
              $_ =~ /^ifndef$/i )
      {
         ##ifdef and ifndef are followed by 1 variable
         $nSkipThisMany = 1;
      }
      elsif ( $_ =~ /^ifdefeq$/i or
              $_ =~ /^ifdefneq$/i )
      {
         ##ifdefeq and ifdefneq are followed by 1 variable and 1 value
         $nSkipThisMany = 2;
      }
      elsif ( not exists $states{ $_ } )
      {
         $bContainsOnlyStates = "false";
      }
   }

   ## if the line contains only states, then we need to see if they are defined
   ## to determine if we should process the following lines.;
   ## if they are all set to true, then we should process the lines.
   ## if the line contains words other than states, we assume it contains a command
   ## and we should let it get processed below
   if ( $bContainsOnlyStates eq "true" )
   {
      $gState = $line;

      Vprint( $OptVerbose, "" );

      my $bDisable = "false";
      my $nWords = @words;
      for( my $i = 0; $i < $nWords; $i++ )
      {
         my $word = $words[$i];
         if ( $word =~ /^ifdef$/i )
         {
            ## ifdef Variable

            ## get Variable
            $i++;
            my $var = $words[$i];

            if ( defined GetVariable( $var ) )
            {
               Vprint( $OptVerbose, "ifdef $var is     true" );
            }
            else
            {
               $bDisable = "true";
               Vprint( $OptVerbose, "ifdef $var is not true" );
            }
         }
         elsif ( $word =~ /^ifndef$/i )
         {
            ## ifndef Variable

            ## get Variable
            $i++;
            my $var = $words[$i];

            if ( not defined GetVariable( $var ) )
            {
               Vprint( $OptVerbose, "ifndef $var is     true" );
            }
            else
            {
               $bDisable = "true";
               Vprint( $OptVerbose, "ifndef $var is not true" );
            }
         }
         elsif ( $word =~ /^ifdefeq$/i )
         {
            ## ifdef Variable "Value"

            ## get Variable
            $i++;
            my $var = $words[$i];

            ## get "Value"
            $i++;
            my $value = $words[$i];

            ##strip off possibly leading and trailing quotation mark
            $value =~ s/^\"(.*)\"$/$1/;

            if ( defined GetVariable( $var ) &&
                 $value eq GetVariable( $var ) )
            {
               Vprint( $OptVerbose, "ifdefeq $var \"$value\" is     true" );
            }
            else
            {
               $bDisable = "true";
               Vprint( $OptVerbose, "ifdefeq $var \"$value\" is not true" );
            }
         }
         elsif ( $word =~ /^ifdefneq$/i )
         {
            ## ifdef Variable "Value"

            ## get Variable
            $i++;
            my $var = $words[$i];

            ## get "Value"
            $i++;
            my $value = $words[$i];

            ##strip off possibly leading and trailing quotation mark
            $value =~ s/^\"(.*)\"$/$1/;
            
            if ( not defined GetVariable( $var ) ||
                 ( defined GetVariable( $var ) &&
                   $value ne GetVariable( $var ) ) )
            {
               Vprint( $OptVerbose, "ifdefneq $var \"$value\" is     true" );
            }
            else
            {
               $bDisable = "true";
               Vprint( $OptVerbose, "ifdefneq $var \"$value\" is not true" );
            }
         }
         elsif ( not $states{ $word } )
         {
            $bDisable = "true";
            Vprint( $OptVerbose, "$word is not defined" );
         }
         else
         {
            Vprint( $OptVerbose, "$word is     defined" );
         }
      }

      Vprint( $OptVerbose, "" );

      if ( $bDisable eq "false" )
      {
         $bProcessLine = "true";
         $gbStateLine = "true";
      }
      else
      {
         $bProcessLine = "false";
      }
   }

   if ( $bProcessLine eq "false" )
   {
      Vprint ( $OptVerbose, "skipping state:'$gState' line: '$line'" );
   }
   else
   {
#      Vprint ( $OptVerbose, "performing state:'$gState' line: '$line'" );

      ## we should be a valid state if we reached this point
      ## All commands are available in all states (provides most flexibility)

      if ( $gbStateLine eq "true" )
      {
         $gbStateLine = "false";
      }
      elsif ( $line =~ /^\s*Define[^a-zA-Z].*/i )
      {
         PerformDefine( $line );
      }
      elsif ( $line =~ /^\s*MakeClient[^a-zA-Z].*/i )
      {
         PerformMakeClient( $line );
      }
      elsif ( $line =~ /^\s*UpdateClient[^a-zA-Z].*/i )
      {
         PerformUpdateClient( $line );
      }
      elsif ( $line =~ /^\s*DeleteClient[^a-zA-Z].*/i )
      {
         PerformDeleteClient( $line );
      }
      elsif ( $line =~ /^\s*Dtk[^a-zA-Z].*/i )
      {
         PerformDtk( $line );
      }
      elsif ( $line =~ /^\s*DtkFork[^a-zA-Z].*/i )
      {
         PerformDtkFork( $line );
      }
      elsif ( $line =~ /^\s*DtkQueue[^a-zA-Z].*/i )
      {
         PerformDtkQueue( $line );
      }
      elsif ( $line =~ /^\s*Sync[^a-zA-Z].*/i )
      {
         PerformSync( $line );
      }
      elsif ( $line =~ /^\s*Update[^a-zA-Z].*/i )
      {
         PerformUpdate( $line )
      }
      elsif ( $line =~ /^\s*P4Add[^a-zA-Z].*/i )
      {
         PerformP4Add( $line );
      }
      elsif ( $line =~ /^\s*CheckOut[^a-zA-Z].*/i )
      {
         PerformCheckOut( $line );
      }
      elsif ( $line =~ /^\s*Version[^a-zA-Z].*/i )
      {
         PerformVersion( $line );
      }
      elsif ( $line =~ /^\s*CheckIn[^a-zA-Z].*/i )
      {
         PerformCheckIn( $line );
      }
      elsif ( $line =~ /^\s*Label[^a-zA-Z].*/i )
      {
         PerformLabel( $line );
      }
      elsif ( $line =~ /^\s*Replace[^a-zA-Z].*/i )
      {
         PerformReplace( $line );
      }
      elsif ( $line =~ /^\s*BuildSln[^a-zA-Z].*/i )
      {
         PerformBuildSln( $line );
      }
      elsif ( $line =~ /^\s*SetEnv[^a-zA-Z].*/i )
      {
         PerformSetEnv( $line );
      }
      elsif ( $line =~ /^\s*Zip[^a-zA-Z].*/i )
      {
         PerformZip( $line );
      }
      elsif ( $line =~ /^\s*SendJenkinsStatus?[^a-zA-Z].*/i )
      {
         PerformSendJenkinsStatus( $line );
      }
      elsif ( $line =~ /^\s*SendBuildErrors?[^a-zA-Z].*/i )
      {
         PerformSendBuildErrors( $line );
      }
      elsif ( $line =~ /^\s*GetTestFilesFromJenkins?[^a-zA-Z].*/i )
      {
         PerformGetTestFilesFromJenkins( $line );
      }
      elsif ( $line =~ /^\s*GetVersionFromLog[^a-zA-Z].*/i )
      {
         PerformGetVersionFromLog( $line );
      }
      elsif ( $line =~ /^\s*ReturnTestResults[^a-zA-Z].*/i )
      {
         PerformReturnTestResults( $line );
      }
      elsif ( $line =~ /^\s*Unzip[^a-zA-Z].*/i )
      {
         PerformUnzip( $line );
      }
      elsif ( $line =~ /^\s*chdir[^a-zA-Z].*/i )
      {
         PerformChdir( $line );
      }
      elsif ( $line =~ /^\s*System[^a-zA-Z].*/i )
      {
         PerformSystem( $line );
      }
      elsif ( $line =~ /^\s*Exec[^a-zA-Z].*/i )
      {
         PerformExec( $line );
      }
      elsif ( $line =~ /^\s*del[^a-zA-Z].*/i )
      {
         PerformDelete( $line );
      }
      elsif ( $line =~ /^\s*Copy[^a-zA-Z].*/i )
      {
         PerformCopy( $line );
      }
      elsif ( $line =~ /^\s*XCopy[^a-zA-Z].*/i )
      {
         PerformXCopy( $line );
      }
      elsif ( $line =~ /^\s*mkdir[^a-zA-Z].*/i )
      {
         PerformMkDir( $line );
      }
      elsif ($line =~ /^\s*Log[^a-zA-Z].*/i ) 
      {
         if (!defined $OptLog)
         {
             PerformLog( $line );
         }
      }
      elsif ( $line =~ /^\s*SDKClean[^a-zA-Z].*/i )
      {
         PerformSdkClean( $line );
      }
      elsif ( $line =~ /^\s*XMLClean[^a-zA-Z].*/i )
      {
         PerformXMLClean( $line );
      }
      elsif ( $line =~ /^\s*Doxygen[^a-zA-Z].*/i )
      {
         PerformDoxygen( $line );
      }
      elsif ( $line =~ /^\s*GenInstall[^a-zA-Z].*/i )
      {
         PerformGenInstall( $line );
      }
      elsif ( $line =~ /^\s*SendInstallErrors[^a-zA-Z].*/i )
      {
         PerformSendInstallErrors( $line );
      }
      elsif ( $line =~ /^\s*BuildMacro[^a-zA-Z].*/i )
      {
         PerformBuildMacro( $line );
      }
      elsif ( $line =~ /^\s*TutorialClean[^a-zA-Z].*/i )
      {
         PerformTutorialClean( $line );
      }
      elsif ( $line =~ /^\s*MakeWriteable[^a-zA-Z].*/i )
      {
         PerformMakeWriteable( $line );
      }
      elsif ( $line =~ /^\s*MakeReadonly[^a-zA-Z].*/i )
      {
         PerformMakeReadOnly( $line );
      }
      elsif ( $line =~ /^\s*VersionUpdateOnTool[^a-zA-Z].*/i )
      {
         PerformVersionUpdateOnTool( $line );
      }
      elsif ( $line =~ /^\s*BrowserUpdate[^a-zA-Z].*/i )
      {
         PerformBrowserUpdate( $line );
      }
      elsif ( $line =~ /^\s*PrependText[^a-zA-Z].*/i )
      {
         PerformPrependText( $line );
      }
      elsif ( $line =~ /^\s*GoogleTest[^a-zA-Z].*/i )
      {
         PerformGoogleTest( $line );
      }
      elsif ( $line =~ /^\s*NUnitTest[^a-zA-Z].*/i )
      {
         PerformNUnitTest( $line );
      }
      elsif ( $line =~ /^\s*WaitForFileExist[^a-zA-Z].*/i )
      {
         PerformWaitForFileExist( $line );
      }
      elsif ( $line =~ /^\s*ReportSystemTime[^a-zA-Z].*/i )
      {
         PerformReportSystemTime( $line );
      }
      elsif ( $line =~ /^\s*SetTimeout[^a-zA-Z].*/i )
      {
         PerformSetTimeout( $line );
      }
      else
      {
         Error( "INVALID KEYWORD at about line number: $currLineNumber - start of line: $line" );
      }
   } # end else valid state
} # End foreach @lines (of configuration file)


################################################################################################################
##                                          Sub Routines start here
################################################################################################################



#######################################
# adds the specified variable with the specified value (replaces nested variables if they exist
#
# Param: name of the variable to create
# Param: value for the variable to have
#
#######################################
sub PerformDefine
{
   my $line = $_[0];

   if ( $line =~ /^\s*Define\s*"([^\"]*)"\s*"([^\"]*)"\s*$/i )
   {
      my $var = $1;
      my $value = $2;

      $value = ProcessForVariables( $value );

      AddVariable( $var, $value );
      Vprint ( $OptVerbose, "Adding variable [$var] = $value" );
   }
   else
   {
      Error( "Bad syntax in Define: '$line'" );
   }
}

#######################################
# Generates an installation file (.msi) based on the specified configuration file
#
# Param: path and name of the configuration file
# Param: working directory which will override those specified in the configuration file
#
#######################################
sub PerformGenInstall
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]+)"\s*$/ or
        $line =~ /^[^\"]*"([^\"]+)"\s*"([^\"]+)"\s*$/ )
   {
      my $configurationFile = $1;

      my $wkDir = getcwd();

      ## move into the home directory
      chdir $gHomeDirectory;

      my $cmd = "GenInstall.bat \"$configurationFile\"";
      my $msg = "GenInstall \"$configurationFile\"";
      if ( defined $2 )
      {
         $msg .= " \"$2\"";
         $cmd .= " \"$2\"";
      }

      Vprint ( LOG, "$msg" );

      ## Handle Version updates
      if ( ( defined GetVariable( "MAJOR" ) ) and
           ( defined GetVariable( "MINOR" ) )
         )
      {
         ## find P4ID that contains the install configuration
         my $p4id;

         my @keys = keys %gP4Settings;
         foreach( @keys )
         {
            my $key = $_;
            my ($root, $client, $server, $user) = GetP4Settings( $key );

            if ( "true" eq P4Have( $client, $server, $user, $configurationFile ) )
            {
               Vprint( $OptVerbose, "$key HAS '$configurationFile'\n" );
                  ## this ensures that we only attempt to use the first client that contains the file
               if ( not defined $p4id )
               {
                  $p4id = $key;
               }
               else
               {
                  ## this will happen if more than one client contains the file
                  Warning( "Both $p4id and $key contain '$configurationFile', but only the perforce connection associated with $p4id will be updated." );
               }
            }
            else
            {
               Vprint( $OptVerbose, "$key does NOT have the file\n" );
            }
         }

         ## rewrite the version number
         ## check out the file from perforce (also makes it not readonly)
         if ( defined $p4id )
         {
            PerformCheckOut( "   CheckOut \"$p4id\" \"$configurationFile\"" );
         }

         my $checkInMsg = "Config file '$configurationFile' was not write-able, could not attempt to update version number or ProductCode.";

         ## file is writeable
         if ( -w $configurationFile )
         {
            my $majorVer = GetVariable( "MAJOR" );
            my $minorVer = GetVariable( "MINOR" );
            my $updateVer = "";
            my $buildVer = "";

            my $replaced = 0;

            if ( ( defined GetVariable( "UPDATE" )) and
                 ( defined GetVariable( "BUILD" ) ) )
            {
               $updateVer = GetVariable( "UPDATE" );
               $buildVer = GetVariable( "BUILD" );
               ## make sure the major, minor, update, and build version match the install configuration
               $replaced = PerformReplace(  "   Replace \"$configurationFile\" \"VERSION: \\\"[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+\\\"\" \"VERSION: \"$majorVer.$minorVer.$updateVer.$buildVer\"\" " );
            }
            else
            {
               ## make sure the major and minor version match the install configuration
               $replaced = PerformReplace(  "   Replace \"$configurationFile\" \"VERSION: \\\"[0-9]+\\.[0-9]+\\\"\" \"VERSION: \"$majorVer.$minorVer\"\" " );
            }


            if ( $replaced > 0 )
            {
               $checkInMsg = "Code version and install version did not match - automatically updating install script with version $majorVer.$minorVer";

               if ( ( defined GetVariable( "UPDATE" )) and
                ( defined GetVariable( "BUILD" ) ) )
           {
                  $checkInMsg .= ".$updateVer.$buildVer";
               }

               ## command line opted for us to replace the ProductID (GUID) 'PRODUCTCODE: "{B1BF208E-33DB-409A-922D-B405679E09E3}"'
               my $newGuid = Win32::GuidGen();
               if ( 0 < PerformReplace(  "   Replace \"$configurationFile\" \"PRODUCTCODE: \\\"\{[A-F0-9]{8}-[A-F0-9]{4}-[A-F0-9]{4}-[A-F0-9]{4}-[A-F0-9]{12}\}\\\"\" \"PRODUCTCODE: \"$newGuid\"\" " ) )
               {
                  $checkInMsg .= "\\nChanged version number caused automatic update to PRODUCTCODE: $newGuid";
               }
               else
               {
                  $checkInMsg .= "\\nFAILED to automatically update PRODUCTCODE, please change it manually.";
               }
            }
            else
            {
               $checkInMsg = "Code version and install version matched, no need to update ProductCode.";
               if ( defined $p4id )
               {
                  $checkInMsg .= " This should have gotten reverted before it was checked in.";
               }
            }
         }

         if ( defined $p4id )
         {
            PerformCheckIn(  "   CheckIn \"$p4id\" \"$checkInMsg\" \"$configurationFile\"" );
         }
         else
         {
            Vprint( LOG, $checkInMsg );
         }
      }

      ## perform the install generation
      MySystem( LOG, "$cmd", "true" );

      my $result = $? >> 8;
      Vprint (LOG, "GenInstall returned: $result\n");
      if ( $result )
      {
         $gInstallFailed += $result;
      }

      ## move back into the working directory
      chdir $wkDir;
   }
   else
   {
      Error( "Bad syntax in GenInstall: '$line'" );
   }
}

#######################################
# Runs the specified playlist as a dtk test in a separate process and emails if there are any failures
#
# Param: name of the playlist to run
# Param: FarmID to run on
# Param: test_output subdirectory
# Param: name of the batch file to copy the output to the wiki
#
#######################################
sub PerformDtkFork
{
   my $line = $_[0];

   my $playlist = "";
   my $farmid = "";
   my $testoutput = "";
   my $wikiBat = "";
   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $playlist = $1;
      $farmid = $2;
      $testoutput = $3;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $playlist = $1;
      $farmid = $2;
      $testoutput = $3;
      $wikiBat = $4;

      if ( not -e $wikiBat )
      {
         Warning( "DtkFork: '$wikiBat' - File does not exist, continuing with testing as if no wikiBat file was specified." );
         $wikiBat = "";
      }
   }
   else
   {
      Error( "Bad syntax in DtkFork: '$_' \n Be sure to uninstall any applications that were installed prior to running Dtk." );
   }

   if ( not -e $playlist )
   {
      Warning( "DTK Playlist file '$playlist' does not exist. The DTK test will not be run." );
      return;
   }

   if ( $farmid eq "" )
   {
      Warning( "DTK FarmID is not set, defaulting to farm 0." );
      $farmid = "0";       
   }

   if ( $testoutput eq "" )
   {
      Warning( "DTK testoutput directory not set, setting output directory to 'Default'.");
      $testoutput = "Default";
   }

   # Fork and perform the DTK testing
   my $pid = fork();
   if ( not defined $pid )
   {
      # not enough memory to spawn the process, should it just get run, or error ?
      Error( "Not enough memory to spawn DTK process" );
   }
   elsif ( $pid == 0 )
   {
      #child process, run DTK test, then exit

      my $buildMachineIP = GetVariable( "DTKIP" );
      my $installRoot = GetVariable( "DTKDIR" );

      if ( defined $installRoot )
      {
         Vprint( $OptVerbose, "DTK InstallRoot: $installRoot" );
      }

      ## get into the installroot directory
      ## and store the current directory so that it can be restored afterwards
      my $wkDir = getcwd();
      chdir "$installRoot";

      ## Refresh DepotServer and give it a little time to react
      MyExec( LOG, "Remote.exe do $buildMachineIP 10003 refresh" );
      sleep( 10 );

      ## determine the correct command line to use.
      my $cmd = "NetConsole.exe -playlist \"$playlist\" -testcoord $buildMachineIP -testoutput \"test_output\\$testoutput\"";
      ## The use of the farmId parameter may contain just the farmId, or it may contain other specific parameters,
      ## so we need to make sure to use it correctly.
      if ( $farmid =~ /^[0-9]+/ )
      {
         #farmID starts with a number, which indicates that it is being used as expected
         $cmd .= " -farmId $farmid";
      }
      else
      {
         #farmID does not start with a number, so its use must be specifying other parameters
         $cmd .= " $farmid";
      }

      ## start NetConsole which runs the tests - the rest of the DTK should already be running
      MySystem( $OptVerbose, "$cmd", "true" );

      ## give NetConsole a few seconds to finish receiving all its data
      sleep( 15 );

      ## restore back to the original working directory
      chdir $wkDir;

      ## parse resulting html file for failures
      open( HTML, "$installRoot/Test_output/$testoutput/html/index.html" );

      chomp( my @lines = <HTML> );

      ## this is an example of a case where there are 2 failures (indicated by 4th to last line) and 0 errors (3rd to last line)
      ##   <TABLE class=details cellSpacing=2 cellPadding=5 width="100%" border=0 ID="Table1">
      ##     <TBODY>
      ##       <TR vAlign=top>
      ##         <TH width="80%" style="BACKGROUND-IMAGE: url('./images/lcorner6.gif'); BACKGROUND-REPEAT: no-repeat">Name</TH>
      ##         <TH>Total</TH>
      ##         <TH>Pass</TH>
      ##         <TH>Failures</TH>
      ##         <TH>Errors</TH>
      ##         <TH noWrap>Not Capable</TH>
      ##         <TH noWrap style="BACKGROUND-IMAGE: url('./images/rcorner6.gif'); BACKGROUND-REPEAT: no-repeat; BACKGROUND-POSITION: top right; text-align: right">Time</TH>
      ##       </TR>    <TR class="b" vAlign=top>
      ##         <TD><A href="Distributed Tests\index.html">Distributed Tests  </A></TD>
      ##         <TD>3&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
      ##         <TD>1&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
      ##         <TD><font color="#FF0000">2</font>&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
      ##         <TD>0&nbsp;&nbsp;&nbsp;&nbsp;</TD>
      ##         ...
      ##       </TR>  </TBODY>
      my $bFoundMachineResults = "false";
      my $nLinesUntilFailures = 14;
      my $nLinesUntilErrors = 15;
      my $bFailed = "true";
      my $bErrored = "true";
      foreach( @lines )
      {
         if ( $bFoundMachineResults eq "true" )
         {
            $nLinesUntilFailures -= 1;
            $nLinesUntilErrors   -= 1;

            ## check for failures
            if ( $nLinesUntilFailures == 0 )
            {
               if ( $_ =~ /^\s*\<TD\>0/ )
               {
                  $bFailed = "false";
               }
            }

            ## check for errors
            if ( $nLinesUntilErrors == 0 )
            {
               if ( $_ =~ /^\s*\<TD\>0/ )
               {
                  $bErrored = "false";
               }
            }

            ## check for end of Table Body
            if ( $_ =~ /\<\/TBODY\>/ )
            {
               $bFoundMachineResults = "false";
            }
         }

         if ( $_ =~ /\<TABLE class\=details cellSpacing\=2 cellPadding\=5 width\=\"100\%\" border\=0 ID\=\"Table1\"\>/ )
         {
            $bFoundMachineResults = "true";
         }
      }

      ## The known path to the index html page in this test output folder
      my $indexURL = "$installRoot/Test_Output/$testoutput/html/index.html";

      ## send email if there is a failure
      if ( $bFailed eq "true" )
      {
         if ( $OptMail )
         {
             if ( not defined GetVariable("MOREDTKOUTPUT") )
             {
                SendEmail( "$OptMail",
                           "Dtk Test Failed",
                           "The Dtk Test Failed for $playlist.\n\nPlease see $indexURL on the build machine for the actual results" );
             }
             else
             {
                SendEmail( "$OptMail",
                           "Dtk Test Failed",
                           "The Dtk Test Failed for $playlist.\n\n" . GetVariable("MOREDTKOUTPUT") );
             }
         }
      }

      ## send email if there is an error
      if ( $bErrored eq "true" )
      {
         if ( $OptMail )
         {
            if ( not defined GetVariable("MOREDTKOUTPUT") )
            {
                SendEmail( "$OptMail",
                           "Dtk Test Encountered an Error",
                           "The Dtk Test Errored for $playlist.\n\nPlease see $indexURL on the build machine for the actual results" );
            }
            else
            {
               SendEmail( "$OptMail",
                          "Dtk Test Encountered an Error",
                          "The Dtk Test Errored for $playlist.\n\n" . GetVariable("MOREDTKOUTPUT") );
            }
         }
      }

      if ( $wikiBat ne "" )
      {
         #replace slashes for index and index_delta
         # this is extra escaped so that we can create the string in Perl, which will get re-evaluated an unescaped to its desired value
         PerformReplace( "Replace \"$installRoot/Test_output/$testoutput/html/index.html\" \"\\\\index\\.html\" \"/index.html\"" );
         PerformReplace( "Replace \"$installRoot/Test_output/$testoutput/html/index.html\" \"\\.\\\\index_delta\\.html\" \"index_delta.html\"" );

         #run wikiBat
         PerformSystem( "System \"$wikiBat\"" );
      }

      # need to exit the child process so that it doesn't stay running!
      exit(0);
   }
   else
   {
      #parent process, just return
   }
}

#######################################
# Queues a DTK test so that Executive can schedule it
#
# Param: name of the playlist to run
# Param: FarmID to run on
# Param: results root directory
# Param: Test identifier
# Param: test Description
# Param: test scheduler
# Param: target test files' directory
#######################################
sub PerformDtkQueue
{
   my $line = $_[0];

   my $playlist = "";
   my $farmid = "";
   my $resultsroot = "";
   my $testID = "";
   my $description = "";
   my $replicate = "";
   my $sourceroot = "";
   my $preset = "Default";

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $playlist = $1;
      $preset = $2;
      $farmid = $3;
      $resultsroot = $4;
      $testID = $5;
      $description = $6;
      $replicate = $7;
      $sourceroot = $8;
      $gResultRoot = $resultsroot;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $playlist = $1;
      $farmid = $2;
      $resultsroot = $3;
      $testID = $4;
      $description = $5;
      $replicate = $6;
      $sourceroot = $7;
      $gResultRoot = $resultsroot;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $playlist = $1;
      $farmid = $2;
      $resultsroot = $3;
      $testID = $4;
      $description = $5;
      $replicate = $6;
      $gResultRoot = $resultsroot;
   }
   else
   {
      Error( "Bad syntax in DtkQueue: '$_' \n Be sure to uninstall any applications that were installed prior to running Dtk." );
   }

   my $BRN         = "";
   my $BRL         = "";
   my $DOP         = "";
   my $testtimeout = $gTestcaseTimeout;
   my $testlimit   = 0;

   if ( not -e $playlist )
   {
      Warning( "DTK Playlist file '$playlist' does not exist. The DTK test will not be run." );
      return;
   }

   if ( $farmid eq "" )
   {
      Warning( "DTK FarmID is not set, DTK test will not be queued." );
      return;
   }

   if ( $resultsroot eq "" )
   {
      Warning( "DTK results root directory not set.");
      return;
   }

   if ( $replicate eq "replicate" )
   {
      $replicate = "1";
   }
   else
   {   
      $replicate = "0";
   }

   ## refresh depotserver
   my $buildMachineIP = GetVariable( "DTKIP" );
   my $installRoot = GetVariable( "DTKDIR" );

   if ( defined $installRoot )
   {
      Vprint( $OptVerbose, "DTK InstallRoot: $installRoot" );
   }

   ## get into the installroot directory
   ## and store the current directory so that it can be restored afterwards
   my $wkDir = getcwd();
   chdir "$installRoot";

   if ($alreadyRefreshedDTKDepot eq "false")
   {
      ## Refresh DepotServer and give it a little time to react
      MyExec( LOG, "Remote.exe do $buildMachineIP 10003 refresh" );

      Vprint( LOG, "Waiting 15 seconds to let depot server refresh..." );
      sleep( 15 );
      Vprint( LOG, "Done." );
      
      $alreadyRefreshedDTKDepot = "true";
   }

   ## if output dir exists, create a folder with similar name;
   my $copies = 1;
   my $testIDCopy =$testID;
   Vprint( LOG, "Making unique test result directory..." );
   while ( -d "$resultsroot\\$testIDCopy\"" )
   {
      $copies++;      
      $testIDCopy = "$testID($copies)";
   }
   $testID = $testIDCopy;
   mkpath("$resultsroot\\$testID");
   Vprint( LOG, "Created $resultsroot\\$testID" );

   push(@gTestResultDIR,$testID);

   if ( $sourceroot ne "" )
   {
      mkdir("$resultsroot\\$testID\\Common");
      PerformXCopy( "xcopy \"$sourceroot\" \"$resultsroot\\$testID\\Common\"" ) || Warning( " target test files not copied.");
   }

   # Create Description.txt 
   open(OUT,">$resultsroot\\$testID\\Description.txt");
   print OUT "User: 3dargbld\n";
   print OUT $description;
   close(OUT);

   my $email = "";
   if ( $OptMail )
   {
      $email = $OptMail;
   }


   # Create test config file
   open(TESTCONF, ">$resultsroot\\$testID\\testing.config");
   {
      printf TESTCONF "//This an auto-generated test config file created by the generic build script\n";
      printf TESTCONF "name =\"global\"\n";
      printf TESTCONF "{\n";
      printf TESTCONF "    TestDescription     = \"$resultsroot\\$testID\\Description.txt\"\n";
      printf TESTCONF "    ResultsEmailAddress = \"$email\"\n";
      printf TESTCONF "    ResultsEmailType    = \"DeltaSummary\"\n";
      printf TESTCONF "    TestID              = \"$testID\"\n";
      printf TESTCONF "    Priority            = \"1000\"\n";
      printf TESTCONF "    EmailJobTitle       = \"GDT Nightly Testing\"\n";
      printf TESTCONF "}\n\n";
      printf TESTCONF "name = \"test1\"\n";
      printf TESTCONF "{\n";
      printf TESTCONF "   TestFarmType       = \"$farmid\"\n";
      printf TESTCONF "   TestLevel          = \"5\"\n";
      printf TESTCONF "   TestRunResultsPath = \"$resultsroot\\$testID\\\"\n";
      printf TESTCONF "   Replicate          = \"$replicate\"\n";
      printf TESTCONF "   Playlist           = \"$playlist\"\n";
      printf TESTCONF "   Preset             = \"$preset\"\n";
      printf TESTCONF "   MiscOptions        = \"\"\n";
      printf TESTCONF "   //DRF - download results on failure\n";
      printf TESTCONF "   DRF                = \"0\"\n";
      printf TESTCONF "   //DOP - Delta Object Path\n";
      printf TESTCONF "   DOP                = \"$DOP\"\n";
      printf TESTCONF "   //BRN - Base Report Name\n";
      printf TESTCONF "   BRN                = \"$BRN\"\n";
      printf TESTCONF "   //BRL - Base Report Hyperlink\n";
      printf TESTCONF "   testtimeout         = \"$testtimeout\"\n";
      printf TESTCONF "   testlimit           = \"$testlimit\"\n";
      printf TESTCONF "   BRL                 = \"$BRL\"\n";
      printf TESTCONF "}\n";
   }
   close(TESTCONF);

}


#######################################
# Runs the specified playlist as a dtk test and emails if there are any failures
#
# Param: name of the playlist to run
#
#######################################
sub PerformDtk
{
   my $line = $_[0];

   my $playlist;
   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      $playlist = $1;
   }
   else
   {
      Error( "Bad syntax in Dtk: '$_' \n Be sure to uninstall any applications that were installed prior to running Dtk." );
   }

   if ( not -e $playlist )
   {
      Warning( "DTK Playlist file '$playlist' does not exist. The DTK test will not be run." );
      return;
   }

   my $buildMachineIP = GetVariable( "DTKIP" );
   my $installRoot = GetVariable( "DTKDIR" );

   if ( defined $installRoot )
   {
      Vprint( $OptVerbose, "DTK InstallRoot: $installRoot" );
   }

   ## get into the installroot directory
   ## and store the current directory so that it can be restored afterwards
   my $wkDir = getcwd();
   chdir "$installRoot";

   ## launch Dtk apps and give them time to initialize
   my $pidDS = MyExec( LOG, "DepotServer.exe" );
   sleep( 5 );
   my $pidTC = MyExec( LOG, "TestCoordinator.exe" );
   sleep( 5 );

   ## start NetConsole which runs the tests - the rest of the DTK should already be running
   my $cmd = "NetConsole.exe -playlist \"$playlist\" -testcoord $buildMachineIP";
   MySystem( $OptVerbose, "$cmd", "true" );

   ## give NetConsole a few seconds to finish receiving all its data
   sleep( 15 );

   MySystem( LOG, "taskkill /IM TestCoordinator.exe", "true" );
   MySystem( LOG, "taskkill /IM DepotServer.exe",     "true" );

   ## restore back to the original working directory
   chdir $wkDir;

   ## parse resulting html file for failures
   open( HTML, "$installRoot/Test_output/html/index.html" );

   chomp( my @lines = <HTML> );

## this is an example of a case where there are 2 failures (indicated by 4th to last line) and 0 errors (3rd to last line)
##   <TABLE class=details cellSpacing=2 cellPadding=5 width="100%" border=0 ID="Table1">
##     <TBODY>
##       <TR vAlign=top>
##         <TH width="80%" style="BACKGROUND-IMAGE: url('./images/lcorner6.gif'); BACKGROUND-REPEAT: no-repeat">Name</TH>
##         <TH>Total</TH>
##         <TH>Pass</TH>
##         <TH>Failures</TH>
##         <TH>Errors</TH>
##         <TH noWrap>Not Capable</TH>
##         <TH noWrap style="BACKGROUND-IMAGE: url('./images/rcorner6.gif'); BACKGROUND-REPEAT: no-repeat; BACKGROUND-POSITION: top right; text-align: right">Time</TH>
##       </TR>    <TR class="b" vAlign=top>
##         <TD><A href="Distributed Tests\index.html">Distributed Tests  </A></TD>
##         <TD>3&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
##         <TD>1&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
##         <TD><font color="#FF0000">2</font>&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
##         <TD>0&nbsp;&nbsp;&nbsp;&nbsp;</TD>
##         ...
##       </TR>  </TBODY>
   my $bFoundMachineResults = "false";
   my $nLinesUntilFailures = 14;
   my $nLinesUntilErrors = 15;
   my $bFailed = "true";
   my $bErrored = "true";
   foreach( @lines )
   {
      if ( $bFoundMachineResults eq "true" )
      {
         $nLinesUntilFailures -= 1;
         $nLinesUntilErrors   -= 1;


         ## check for failures
         if ( $nLinesUntilFailures == 0 )
         {
            if ( $_ =~ /^\s*\<TD\>0/ )
            {
               $bFailed = "false";
            }
         }

         ## check for errors
         if ( $nLinesUntilErrors == 0 )
         {
            if ( $_ =~ /^\s*\<TD\>0/ )
            {
               $bErrored = "false";
            }
         }

         ## check for end of Table Body
         if ( $_ =~ /\<\/TBODY\>/ )
         {
            $bFoundMachineResults = "false";
         }
      }

      if ( $_ =~ /\<TABLE class\=details cellSpacing\=2 cellPadding\=5 width\=\"100\%\" border\=0 ID\=\"Table1\"\>/ )
      {
         $bFoundMachineResults = "true";
      }
   }


   ## if error or failure, copy html to a shared directory
   my $indexURL = "$installRoot/Test_Output/html/index.html";
   if ( $bFailed eq "true" or
        $bErrored eq "true" )
   {
      my $sharedFolder = "C:\\";

      if ( not defined GetVariable("AUTOBUILDDIR") )
      {
         $sharedFolder = GetVariable("AUTOBUILDDIR");
      }

      $sharedFolder .= "Shared";

      if ( -e $sharedFolder )
      {
         Vprint( $OptVerbose, "Shared directory exists at $sharedFolder" );

         $indexURL = "$sharedFolder\\";

         if ( $playlist =~ /([^\/\\]+)\.plb$/ )
         {
            $indexURL .= "$1\\$gDate\\";
         }
         else
         {
            $indexURL .= "dtkResults\\$gDate\\";
         }

         ## recreate directory
         PerformDelete( "Del \"$indexURL\"" );
         PerformMkDir( "MkDir \"$indexURL\"" );

         ## copy new results
         PerformXCopy( "xcopy \"$installRoot\\Test_Output\" \"$indexURL\"" );

         ## make URL from a Path
         $indexURL =~ s/C:/\\\\ma_3dargbld/;
         $indexURL .= "html\\index.html";
      }
      else
      {
         Vprint( $OptVerbose, "Shared directory does not exist at $sharedFolder; not copying dtk results" );
      }
   }

   ## send email if there is a failure
   if ( $bFailed eq "true" )
   {
      if ( $OptMail )
      {
          if ( not defined GetVariable("MOREDTKOUTPUT") )
          {
             SendEmail( "$OptMail",
                        "Dtk Test Failed",
                        "The Dtk Test Failed for $playlist.\n\nPlease see $indexURL on the build machine for the actual results" );
          }
          else
          {
             SendEmail( "$OptMail",
                        "Dtk Test Failed",
                        "The Dtk Test Failed for $playlist.\n\n" . GetVariable("MOREDTKOUTPUT") );
          }
      }
   }

   ## send email if there is an error
   if ( $bErrored eq "true" )
   {
      if ( $OptMail )
      {
          if ( not defined GetVariable("MOREDTKOUTPUT") )
          {
             SendEmail( "$OptMail",
                        "Dtk Test Encountered an Error",
                        "The Dtk Test Errored for $playlist.\n\nPlease see $indexURL on the build machine for the actual results" );
          }
          else
          {
             SendEmail( "$OptMail",
                        "Dtk Test Encountered an Error",
                        "The Dtk Test Errored for $playlist.\n\n" . GetVariable("MOREDTKOUTPUT") );
          }
      }
   }
}

#######################################
# Sets an environment variable
#
# Param: name of the variable
# Param: Value to set it to
#
#######################################
sub PerformSetEnv
{
   my $line = $_[0];
   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      my $variable = $1;
      my $value = $2;

      Vprint( LOG, "Set Environment Variable $variable = $value" );

      my $origValue = $ENV{$variable};

      ## need to check if $value contains any existing environment variables and replace them with their value
      while ( $value =~ /(.*)\%([^\%]+)\%(.*)/ )
      {
         my $start = $1;
         my $tmpVar = $2;
         my $end = $3;

         my $tmpVal = $ENV{$tmpVar};

         $value = "$1$tmpVal$3";
      }

      $ENV{$variable} = $value;

      my $newValue = $ENV{$variable};

      unless ( defined $origValue )
      {
         $origValue = "undefined";
      }

      Vprint( $OptVerbose, "  Original value: $origValue" );
      Vprint( LOG, "  New value: $newValue" );
   }
   else
   {
      Error( "Bad syntax in SetEnv: '$_'" );
   }
}

#######################################
# Package this source into a zip file
#
# Param: name of the zipfile to create
# Param: directory to zip
#
#######################################
sub PerformZip
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      my $zipName = $1;
      my $directory = $2;

      Vprint( LOG, "Creating Zipfile: $zipName" );

      my $dirName = basename $directory;

      Vprint( LOG, "DirName: $dirName" );
      Vprint( LOG, "Directory: $directory" );

      my $zip = Archive::Zip->new();
      $zip->addTree( "$directory", "$dirName/") == AZ_OK or die "add tree error: $!";
      die 'zip write error: $!' unless $zip->writeToFileNamed("$zipName") == AZ_OK;
   }
   else
   {
      Error( "Bad syntax in Zip: '$line'" );
   }
}

######################################
# Copies the doxygen exe to the specifed directory
# Generated documentation based on the specified configuration file
#
# Param: string The configuration file to use
# Param: string The directory to generate the documentation in
#
######################################
sub PerformDoxygen
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      my $doxygenFile = $1;
      my $outputDir = $2;

      unless ( -d $outputDir )
      {
         Error ( "Invalid output directory: $outputDir" );
      }

      unless ( -e "$gHomeDirectory\\..\\doxygen\\bin\\doxygen.exe" and
               -e "$gHomeDirectory\\..\\doxygen\\bin\\qt-mt230nc.dll" )
      {
         Error( "Cannot perform 'doxygen': necessary files are not synced from perforce" );
      }

      copy( "$gHomeDirectory\\..\\doxygen\\bin\\doxygen.exe", $outputDir );
      copy( "$gHomeDirectory\\..\\doxygen\\bin\\qt-mt230nc.dll", $outputDir );

      MySystem( $OptVerbose, "$outputDir\\doxygen.exe \"$doxygenFile\"" );

      unlink "$outputDir\\doxygen.exe";
      unlink "$outputDir\\qt-mt230nc.dll";
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*\s*"([^\"]*)"$/ )
   {
      my $doxygenDir = $1;
      my $doxygenFile = $2;
      my $outputDir = $3;

      unless ( -d $outputDir )
      {
         Error ( "Invalid output directory: $outputDir" );
      }

      unless ( -e "$doxygenDir\\bin\\doxygen.exe" )
      {
         Error( "Cannot perform 'doxygen': doxygen.exe is not in the specified doxygen directory '$doxygenDir'" );
      }

      my @matchingFiles;
      my $cwd = getcwd();

      if ( -d "$doxygenDir\\bin" )
      {
         chdir "$doxygenDir\\bin";
      }
      push ( @matchingFiles, bsd_glob("*.dll") );
      chdir "$cwd";

      my $nmatches = @matchingFiles;

      if ( $nmatches == 0 )
      {
         Error( "Cannot perform 'doxygen': necessary dlls are not in the specified doxygen directory '$doxygenDir'" );
      }

      copy( "$doxygenDir\\bin\\doxygen.exe", $outputDir );
      copy( "$doxygenDir\\bin\\*.dll", $outputDir );

      MySystem( $OptVerbose, "$doxygenDir\\bin\\doxygen.exe \"$doxygenFile\"" );

      unlink "$outputDir\\doxygen.exe";
      foreach ( @matchingFiles )
      {
         unlink "$outputDir\\$_";
      }
   }
   else
   {
      Error( "Bad syntax in Doxygen: '$line'" );
   }

   ### copy documentation to web server directory
   ##    if ($OptDocumentation)
   ##    {
   ##        chdir $P4Root;  # required as scp command doesn't like "c:\" in path
   ##
   ##        my $DestServer = '3dargbld@matrix.atitech.com';
   ##
   ##        my $SrcDir = "./3darg/Demos/Sushi3/SourceDoc/html";
   ##        my $DestDir = "/Library/WebServer/Documents/SushiDoxygen";
   ##        Vprint (LOG, "copying documentation to $DestServer:$DestDir");
   ##
   ##        MySystem($OptVerbose, "ssh $DestServer \"find $DestDir -type f -exec rm {} \\; \"");
   ##        MySystem($OptVerbose, "scp -r $SrcDir/* $DestServer:$DestDir");
   ##        MySystem($OptVerbose, "ssh $DestServer \"find $DestDir -type f -exec chmod 0755 {} \\; \"");
   ##    }
}

######################################
# Removes a block of xml that starts with the beginning of the specified tag
#
# Param: string A path to the file or files that must be cleaned (may use wildcard)
# Param: string The start of the XML tag to be removed.
#
######################################
sub PerformXMLClean
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"(.*)"\s*$/ )
   {
      my $directory = $1;
      my $xmlTag = $2;

      my $setVerbose = "";
      if ( $OptVerbose )
      {
         $setVerbose = "--verbose";
      }

      $xmlTag =~ s/\"/\\\"/g; #escape "

      Vprint( LOG, "XMLClean \"$directory\" \"$xmlTag\"" );
      MySystem( $OptVerbose, "\"$gHomeDirectory\\Resources\\xmlclean.pl\" $setVerbose --source \"$directory\" --tag \"$xmlTag\"" );
   }
}

######################################
# Removes Source control settings and build configurations from Visual Studio files
#
# Param: string A path to a directory which must be cleaned
# Param: string A list of build configurations to remove (separated by '|' )
#
######################################
sub PerformSdkClean
{
   my $line = $_[0];

   my $directory = "";
   my $buildTargets = "";
   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
      $directory = $1;
      $buildTargets = $2;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      $directory = $1;
   }
   else
   {
      Error( "Bad syntax in SDKClean: '$line'" );
   }

   my $setVerbose = "";
   if ( $OptVerbose )
   {
      $setVerbose = "--verbose";
   }

   Vprint( LOG, "SDKClean \"$directory\" \"$buildTargets\"" );
   MySystem( $OptVerbose, "\"$gHomeDirectory\\Resources\\sdkclean.pl\" $setVerbose --source \"$directory\" --builds \"$buildTargets\"" );
}

######################################
# Updates the file based on the lines containing version numbers
# Build Version is incremented if 'version' flag is set on command line
#
# Param: string A path to a file or files (using wildcard)
# Param: string The line containing a version number that needs to be read or modified
#
######################################

## NOTE: This code assumes that either three (in the order of [MAJOR], [MINOR], [BUILD])
##                                           (in the order of [MAJOR], [MINOR], [UPDATE])
##                                     four  (in the order of [MAJOR], [MINOR], [UPDATE], [BUILD])
##                                           (in the order of [MAJOR], [MINOR], [BUILD], [UPDATE])
##                                  or one are present in the line
sub PerformVersion
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
      my $filename = $1;
      my $expression = $2;

      my $majorLoc  = -1;
      my $minorLoc  = -1;
      my $buildLoc  = -1;
      my $updateLoc = -1;

      ## check to see if the script specifies to update the version variables
      ## or if they should be updated based on the command-line parameters
      my $nIncrementMajorVersion = 0;
      my $nIncrementMinorVersion = 0;
      my $nIncrementBuildVersion = 0;
      my $nIncrementUpdateVersion = 0;

      if ( $expression =~ /\[GETMAJOR\+\+\]/ or
           $OptMajorVersion )
      {
         $expression =~ s/\[GETMAJOR\+\+\]/\[GETMAJOR\]/;
         $nIncrementMajorVersion = 1;
      }
      if ( $expression =~ /\[GETMINOR\+\+\]/ or
           $OptMinorVersion )
      {
         $expression =~ s/\[GETMINOR\+\+\]/\[GETMINOR\]/;
         $nIncrementMinorVersion = 1;
      }
      if ( $expression =~ /\[GETBUILD\+\+\]/ or
           $OptBuildVersion )
      {
         $expression =~ s/\[GETBUILD\+\+\]/\[GETBUILD\]/;
         $nIncrementBuildVersion = 1;
      }
      if ( $expression =~ /\[GETUPDATE\+\+\]/ or
            $OptUpdateVersion )
      {
         $expression =~ s/\[GETUPDATE\+\+\]/\[GETUPDATE\]/;
         $nIncrementUpdateVersion = 1;
      }

      ## get location of four version components
      $majorLoc  = index( $expression, "[GETMAJOR]" );
      $minorLoc  = index( $expression, "[GETMINOR]" );
      $buildLoc  = index( $expression, "[GETBUILD]" );
      $updateLoc = index( $expression, "[GETUPDATE]" );

      my $exprPreMajor;
      my $exprPreMinor;
      my $exprPreBuild;
      my $exprPreUpdate;
      my $exprEnd;

      ## if all four version numbers are in the string,
      ## we need to make sure we access them in the correct order
      if (   ( $majorLoc > -1 )
         and ( $minorLoc > -1 )
         and ( $buildLoc > -1 )
         and ( $updateLoc > -1 ) )
      {
         if ( $expression =~ /^(.*)\[GETMAJOR\](.*)\[GETMINOR\](.*)\[GETUPDATE\](.*)\[GETBUILD](.*)$/ )
         {
            $exprPreMajor = $1;
            $exprPreMinor = $2;
            $exprPreUpdate = $3;
            $exprPreBuild = $4;
            $exprEnd = $5;
         }
         elsif ( $expression =~ /^(.*)\[GETMAJOR\](.*)\[GETMINOR\](.*)\[GETBUILD](.*)\[GETUPDATE\](.*)$/ )
         {
            $exprPreMajor = $1;
            $exprPreMinor = $2;
            $exprPreBuild = $3;
            $exprPreUpdate = $4;
            $exprEnd = $5;
         }
         else
         {
            Error( "Version expression must contain either one, two (MAJOR,MINOR), three (MAJOR,MINOR,BUILD or MAJOR,MINOR,UPDATE), or all four (MAJOR.MINOR.UPDATE.BUILD or MAJOR.MINOR.BUILD.UPDATE) version numbers" );
         }
      }
      elsif (   ( $majorLoc > -1 )
            and ( $minorLoc > -1 )
            and ( $buildLoc > -1 )
            and ( $updateLoc == -1 ) )
      {
         # line only has Major, Minor, and Build numbers
         if ( $expression =~ /^(.*)\[GETMAJOR\](.*)\[GETMINOR\](.*)\[GETBUILD](.*)$/ )
         {
            $exprPreMajor = $1;
            $exprPreMinor = $2;
            $exprPreBuild = $3;
            $exprEnd = $4;
         }
         else
         {
            Error( "Version expression must contain either one, two (MAJOR,MINOR), three (MAJOR,MINOR,BUILD or MAJOR,MINOR,UPDATE), or all four (MAJOR.MINOR.UPDATE.BUILD or MAJOR.MINOR.BUILD.UPDATE) version numbers" );
         }
      }
      elsif (   ( $majorLoc > -1 )
            and ( $minorLoc > -1 )
            and ( $buildLoc == -1 )
            and ( $updateLoc > -1 ) )
      {
         # line only has Major, Minor, and Update numbers
         if ( $expression =~ /^(.*)\[GETMAJOR\](.*)\[GETMINOR\](.*)\[GETUPDATE](.*)$/ )
         {
            $exprPreMajor = $1;
            $exprPreMinor = $2;
            $exprPreUpdate = $3;
            $exprEnd = $4;
         }
         else
         {
            Error( "Version expression must contain either one, two (MAJOR,MINOR), three (MAJOR,MINOR,BUILD or MAJOR,MINOR,UPDATE), or all four (MAJOR.MINOR.UPDATE.BUILD or MAJOR.MINOR.BUILD.UPDATE) version numbers" );
         }
      }
      elsif (   ( $majorLoc > -1 )
            and ( $minorLoc > -1 )
            and ( $buildLoc == -1 )
            and ( $updateLoc == -1 ) )
      {
         # line only has Major, and Minor
         if ( $expression =~ /^(.*)\[GETMAJOR\](.*)\[GETMINOR\](.*)$/ )
         {
            $exprPreMajor = $1;
            $exprPreMinor = $2;
            $exprEnd = $4;
         }
         else
         {
            Error( "Version expression must contain either one, two (MAJOR,MINOR), three (MAJOR,MINOR,BUILD or MAJOR,MINOR,UPDATE), or all four (MAJOR.MINOR.UPDATE.BUILD or MAJOR.MINOR.BUILD.UPDATE) version numbers" );
         }
      }
      else
      {
         ## we're assuming only one of the three are in the string
         if ( $expression =~ /^(.*)\[GETMAJOR\](.*)$/ )
         {
            $exprPreMajor = $1;
            $exprEnd = $2;
         }
         elsif ( $expression =~ /^(.*)\[GETMINOR\](.*)$/ )
         {
            $exprPreMinor = $1;
            $exprEnd = $2;
         }
         elsif ( $expression =~ /^(.*)\[GETBUILD\](.*)$/ )
         {
            $exprPreBuild = $1;
            $exprEnd = $2;
         }
         elsif ( $expression =~ /^(.*)\[GETUPDATE\](.*)$/ )
         {
            $exprPreUpdate = $1;
            $exprEnd = $2;
         }
         else
         {
            Error( "Version expression doesn't contain any version numbers" );
         }
      }

      ## at this point, the expression has been updated, so format it to be a regular expression
      $expression =~ s/\./\\./g; # replace "." with "\."
      $expression =~ s/\(/\\(/g; # replace "(" with "\("
      $expression =~ s/\)/\\)/g; # replace ")" with "\)"
      $expression =~ s/\[GETMAJOR\]/([0-9]+)/g;
      $expression =~ s/\[GETMINOR\]/([0-9]+)/g;
      $expression =~ s/\[GETUPDATE\]/([0-9]+)/g;
      $expression =~ s/\[GETBUILD\]/([0-9]+)/g;

      my $regexp = qr/(.*)$expression(.*)/;

      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$filename") );

      foreach ( @matchingFiles )
      {
         my $match = $_;

         Vprint( LOG, "Updating version number in $match" );

         open( FILEIN, $match ) or
            Error( "Cannot open file: $match for reading: $!" );

         chomp (my @lines = <FILEIN>);
         close FILEIN;

         my $majorVersion;
         my $minorVersion;
         my $updateVersion;
         my $buildVersion;

         foreach( @lines )
         {
            if ( $_ =~ /$regexp/ )
            {
               Vprint ( $OptVerbose, "OLD LINE: $_" );
               my $buildNum;
               if (   ( $majorLoc > -1 )
                  and ( $minorLoc > -1 )
                  and ( $updateLoc > -1 )
                  and ( $buildLoc > -1 ) )
               {
                  ## this line contains all four version numbers
                  ## so rewrite it
                  $majorVersion = $2;
                  $minorVersion = $3;

                  if ( $updateLoc < $buildLoc )
                  {
                     $updateVersion = $4;
                     $buildVersion = $5;
                  }
                  else
                  {
                     $updateVersion = $5;
                     $buildVersion = $4;
                  }

                  ## increment version numbers as needed
                  if ( $nIncrementMajorVersion or
                       $nIncrementMinorVersion or
                       $nIncrementBuildVersion or
                       $nIncrementUpdateVersion )
                  {
                     $majorVersion += $nIncrementMajorVersion;
                     $minorVersion += $nIncrementMinorVersion;
                     $buildVersion += $nIncrementBuildVersion;
                     $updateVersion += $nIncrementUpdateVersion;

                     if ( $updateLoc < $buildLoc )
                     {
                        Vprint (LOG, "Version number increased to $majorVersion.$minorVersion.$updateVersion.$buildVersion");
                     }
                     else
                     {
                        Vprint (LOG, "Version number increased to $majorVersion.$minorVersion.$buildVersion.$updateVersion");
                     }
                  }
                  else
                  {
                     if ( $updateLoc < $buildLoc )
                     {
                        Vprint (LOG, "Version number remains at $majorVersion.$minorVersion.$updateVersion.$buildVersion");
                     }
                     else
                     {
                        Vprint (LOG, "Version number remains at $majorVersion.$minorVersion.$buildVersion.$updateVersion");
                     }
                  }

                  ## Update the line with the new version numbers
                  if ( $updateLoc < $buildLoc )
                  {
                     $_ = "$1$exprPreMajor$majorVersion$exprPreMinor$minorVersion$exprPreUpdate$updateVersion$exprPreBuild$buildVersion$exprEnd$6";
                  }
                  else
                  {
                     $_ = "$1$exprPreMajor$majorVersion$exprPreMinor$minorVersion$exprPreBuild$buildVersion$exprPreUpdate$updateVersion$exprEnd$6";
                  }
               }
               elsif (   ( $majorLoc > -1 )
                     and ( $minorLoc > -1 )
                     and ( $buildLoc > -1 )
                     and ( $updateLoc == -1 ) )
               {
                  ## this line contains Major, Minor, and Build numbers
                  ## so rewrite it
                  $majorVersion = $2;
                  $minorVersion = $3;
                  $buildVersion = $4;

                  if ( $nIncrementMajorVersion or
                       $nIncrementMinorVersion or
                       $nIncrementBuildVersion )
                  {
                     $majorVersion += $nIncrementMajorVersion;
                     $minorVersion += $nIncrementMinorVersion;
                     $buildVersion += $nIncrementBuildVersion;
                     Vprint (LOG, "Version number increased to $majorVersion.$minorVersion.$buildVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Version number remains at $majorVersion.$minorVersion.$buildVersion");
                  }

                  ## Update the line with the new version numbers
                  $_ = "$1$exprPreMajor$majorVersion$exprPreMinor$minorVersion$exprPreBuild$buildVersion$exprEnd$5";
               }
               elsif (   ( $majorLoc > -1 )
                     and ( $minorLoc > -1 )
                     and ( $buildLoc == -1 )
                     and ( $updateLoc > -1 ) )
               {
                  ## this line contains Major, Minor, and Update numbers
                  ## so rewrite it
                  $majorVersion = $2;
                  $minorVersion = $3;
                  $updateVersion = $4;

                  if ( $nIncrementMajorVersion or
                       $nIncrementMinorVersion or
                       $nIncrementUpdateVersion )
                  {
                     $majorVersion += $nIncrementMajorVersion;
                     $minorVersion += $nIncrementMinorVersion;
                     $updateVersion += $nIncrementUpdateVersion;
                     Vprint (LOG, "Version number increased to $majorVersion.$minorVersion.$updateVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Version number remains at $majorVersion.$minorVersion.$updateVersion");
                  }

                  ## Update the line with the new version numbers
                  $_ = "$1$exprPreMajor$majorVersion$exprPreMinor$minorVersion$exprPreUpdate$updateVersion$exprEnd$5";
               }
               elsif (   ( $majorLoc > -1 )
                     and ( $minorLoc > -1 )
                     and ( $buildLoc == -1 )
                     and ( $updateLoc == -1 ) )
               {
                  ## this line contains Major, and Minor numbers
                  ## so rewrite it
                  $majorVersion = $2;
                  $minorVersion = $3;

                  if ( $nIncrementMajorVersion or
                       $nIncrementMinorVersion )
                  {
                     $majorVersion += $nIncrementMajorVersion;
                     $minorVersion += $nIncrementMinorVersion;
                     Vprint (LOG, "Version number increased to $majorVersion.$minorVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Version number remains at $majorVersion.$minorVersion");
                  }

                  ## Update the line with the new version numbers
                  $_ = "$1$exprPreMajor$majorVersion$exprPreMinor$minorVersion$exprEnd$5";
               }
               elsif ( $buildLoc > -1 )
               {
                  ## only the build number is on this line
                  $buildVersion = $2;
                  if ( $nIncrementBuildVersion )
                  {
                     $buildVersion += $nIncrementBuildVersion;
                     Vprint (LOG, "Build number increased from $2 to $buildVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Build number remains at $buildVersion");
                  }
                  $_ = "$1$exprPreBuild$buildVersion$exprEnd$3";
               }
               elsif ( $majorLoc > -1 )
               {
                  ## only the Major number is on this line
                  $majorVersion = $2;
                  if ( $nIncrementMajorVersion )
                  {
                     $majorVersion += $nIncrementMajorVersion;
                     Vprint (LOG, "Major version increased from $2 to $majorVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Major version remains at $majorVersion");
                  }
                  $_ = "$1$exprPreMajor$majorVersion$exprEnd$3";
               }
               elsif ( $updateLoc > -1 )
               {
                  ## only the Update number is on this line
                  $updateVersion = $2;
                  if ( $nIncrementUpdateVersion )
                  {
                     $updateVersion += $nIncrementUpdateVersion;
                     Vprint (LOG, "Update version increased from $2 to $updateVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Update version remains at $updateVersion");
                  }
                  $_ = "$1$exprPreUpdate$updateVersion$exprEnd$3";
               }
               elsif ( $minorLoc > -1 )
               {
                  ## only the minor number is on this line
                  $minorVersion = $2;
                  if ( $nIncrementMinorVersion )
                  {
                     $minorVersion += $nIncrementMinorVersion;
                     Vprint (LOG, "Minor version increased from $2 to $minorVersion");
                  }
                  else
                  {
                     Vprint (LOG, "Minor version remains at $minorVersion");
                  }
                  $_ = "$1$exprPreMinor$minorVersion$exprEnd$3";
               }
               else
               {
                  Error( "Version regex '$regexp' matches, but could not be rewritten" );
               }
               Vprint ( $OptVerbose, "NEW LINE: $_" );
            }
         }

         if ( defined $majorVersion )
         {
            AddVariable( "MAJOR", "$majorVersion" );
            $gVersion =~ s/^.*\.(.*)\.(.*)\.(.*)$/$majorVersion\.$1\.$2\.$3/;
         }
         if ( defined $minorVersion )
         {
            AddVariable( "MINOR", "$minorVersion" );
            $gVersion =~ s/^(.*)\..*\.(.*)\.(.*)$/$1\.$minorVersion\.$2\.$3/;
         }
         if ( defined $buildVersion )
         {
            AddVariable( "BUILD", "$buildVersion" );
            $gVersion =~ s/^(.*)\.(.*)\..*\.(.*)$/$1\.$2\.$buildVersion\.$3/;
         }
         if ( defined $updateVersion )
         {
            AddVariable( "UPDATE", "$updateVersion" );
            $gVersion =~ s/^(.*)\.(.*)\.(.*)\..*$/$1\.$2\.$3\.$updateVersion/;
         }

         ## only try to save the file back out if a version number will be updated ( incremented )
         ## otherwise just leave it how it was
         if ( $nIncrementMajorVersion  > 0 or
              $nIncrementMinorVersion  > 0 or
              $nIncrementUpdateVersion > 0 or
              $nIncrementBuildVersion  > 0 )
         {
            open( FILEOUT, ">$match" ) or Error( "Could not open $match for writing. Please confirm that it has been checked out of the tree." );

            foreach( @lines)
            {
               print FILEOUT "$_\n";
            }
            close FILEOUT;
         }

      }

      ## update the [VERSION] variable
      if ( defined GetVariable( "MAJOR" ) and
           defined GetVariable( "MINOR" ) and
           defined GetVariable( "BUILD" ) and
           defined GetVariable( "UPDATE" ) )
      {
         my $major = GetVariable( "MAJOR" );
         my $minor = GetVariable( "MINOR" );
         my $build = GetVariable( "BUILD" );
         my $update = GetVariable( "UPDATE" );
         AddVariable( "VERSION", "$major.$minor.$build.$update" );
      }
      elsif ( $gVersion !~ /\[MAJOR\]/ and
              $gVersion !~ /\[MINOR\]/ and
              $gVersion !~ /\[BUILD\]/ and
              $gVersion !~ /\[UPDATE\]/ )
      {
         AddVariable( "VERSION", "$gVersion" );
      }
      
   }
   else
   {
      Error( "Bad syntax in Version: '$line'" );
   }
}

######################################
# Changes directory based on the argument
#
# Param: string A path to change the working directory to
#
######################################
sub PerformChdir
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      my $path = $1;
      chdir "$path";
      Vprint( LOG, "Chdir $path" );
   }
}

########################################
# Send build log and build status to the
# Jenkins Dashboard
#
# Param: The Jenkins project name
#
########################################
sub PerformSendJenkinsStatus
{
  my $line = $_[0];

  if ( $line =~ /^[^\"]*"(.*)"\s*$/ )
  { 
     Vprint (LOG, "Sending results to Jenkins");

     # convert any spaces in the supplied project name to %20
     # to make it HTML post friendly
     my $jenkinsProject = $1;
     $jenkinsProject =~ s/\s/%20/g;
     
     # Jenkins requires logfile to be presented in hexbinary format. 
     # Read & convert
     my $jenkinsHex;
     open (INFILE, $gLogFile);

     foreach (<INFILE>)
     {
        $jenkinsHex .= unpack("H*", $_);
     }
     close (INFILE);

     # time since build started - logged by Jenkins in milliseconds
     my $jenkinsTime = (time - $gBuildStartTime) * 1000;

     Vprint (LOG, "Build time sent to Jenkins = $jenkinsTime");

     # Jenkins defined XML format string that contains build log and build status
     my $jenkinsXML = "<run><log encoding = \"HexBinary\">$jenkinsHex</log><result>$gBuildFailed</result><duration>$jenkinsTime</duration></run>";

     # send to Jenkins
     my $response = post("$gJenkinsURL/job/$jenkinsProject/postBuildResult", "$jenkinsXML"); 
   }
   else
   {
      Error( "Bad syntax in SendJenkinsStatus: '$line'" );
   }
}

######################################
# Emails the specified recipients if any of the previous builds have failed
#
# Param: string A list of email addresses to notify
# Param: string The subject of the email
# Param: string Path to the build log
#
######################################
sub PerformSendBuildErrors
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
      my $recipient = $1;
      my $subject = $2;

      ## send build status email
      if ( $gBuildFailed or $gBuildFailedMessage ne "" )
      {
         Vprint (LOG, "emailing Visual Studio failure to $recipient");
         SendEmail( "$recipient",
                    "$subject",
                    "The attached file contains the complete build log.\n\nThe following build targets failed:\n$gBuildFailedMessage",
                    "$gLogFile" );

         ## reset the build information
         $gBuildFailed = 0;
         $gBuildFailedMessage = "";

         Vprint( LOG, "Build failed, exiting" );
         exit 1;
      }
      else
      {
         Vprint( LOG, "Build successful, no errors to report" );
      }
   }
   else
   {
      Error( "Bad syntax in SendBuildErrors: '$line'" );
   }
}



###########################################
# Get the test files from Jenkins
# 
#
# Param: The zipfile name
# Param: The path for the archived file
# Param: Jenkins Project Name
###########################################
sub PerformGetTestFilesFromJenkins
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*"(.*)"\s*"(.*)"\s*$/ )
   { 
     my $file = "$1";
     my $installRoot = $2;
     my $jenkinsProject = $3;
     my $destFile = $4;
     $jenkinsProject =~ s/\s/%20/g;
     my $url = "$gJenkinsURL/job/$jenkinsProject/lastSuccessfulBuild/artifact/$installRoot";

     my $status = getstore( "$url/$file", "$destFile" );

     Vprint( LOG, "Copying $file from $url to $destFile resulted in http status $status." );   
   }
   else
   {
      Error( "Bad syntax in GetTestFilesFromLog: '$line'" );
   }
}

###########################################
# Get the test files from Jenkins
# 
#
# Param: The zipfile name
# Param: The dest folder for unzipped files
###########################################
sub PerformUnzip
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
     my $file = "$1";     
     my $destDir = $2;
     
     Vprint( LOG, "unZipfile: $file" );

     Vprint( LOG, "Directory: $destDir" );

      my $extractArchive = Archive::Extract->new( archive => "$file" );

      my $ok = $extractArchive->extract( to => $destDir );
  
      PerformDelete( "Del \"$file\"" );  
     
   }
   else
   {
      Error( "Bad syntax in unzip: '$line'" );
   }
}




########################################
# Send Test Results
#
# Param: "DeltaSummary" or "ResultReport"
#
########################################
sub PerformReturnTestResults
{
  my $line = $_[0];

  if ( $line =~ /^[^\"]*"(.*)"\s*$/ )
  { 

     my $resultDir;
     foreach $resultDir (@gTestResultDIR)
     {
        if ( -d "$gResultRoot\\$resultDir" )
        {
           Vprint (LOG, "Waiting for the results to be generated here: $gResultRoot\\$resultDir\\test_output\\html\\index.html\n");
           sleep( 1 ) while ( !(-e "$gResultRoot\\$resultDir\\test_output\\html\\index.html" ) );
  
           Vprint (LOG, "Results found.\n");
           open( HTML, "$gResultRoot\\$resultDir\\test_output\\html\\index.html" );

           chomp( my @lines = <HTML> );
  
           ## this is an example of a case where there are 2 failures (indicated by 4th to last line) and 0 errors (3rd to last line)
           ##   <TABLE class=details cellSpacing=2 cellPadding=5 width="100%" border=0 ID="Table1">
           ##     <TBODY>
           ##       <TR vAlign=top>
           ##         <TH width="80%" style="BACKGROUND-IMAGE: url('./images/lcorner6.gif'); BACKGROUND-REPEAT: no-repeat">Name</TH>
           ##         <TH>Total</TH>
           ##         <TH>Pass</TH>
           ##         <TH>Failures</TH>
           ##         <TH>Errors</TH>
           ##         <TH noWrap>Not Capable</TH>
           ##         <TH noWrap style="BACKGROUND-IMAGE: url('./images/rcorner6.gif'); BACKGROUND-REPEAT: no-repeat; BACKGROUND-POSITION: top right; text-align: right">Time</TH>
           ##       </TR>    <TR class="b" vAlign=top>
           ##         <TD><A href="Distributed Tests\index.html">Distributed Tests  </A></TD>
           ##         <TD>3&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
           ##         <TD>1&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
           ##         <TD><font color="#FF0000">2</font>&nbsp;&nbsp;&nbsp;&nbsp;      <img border="0" src="./images/deltaplus.gif" width="15" height="8"></TD>
           ##         <TD>0&nbsp;&nbsp;&nbsp;&nbsp;</TD>
           ##         ...
           ##       </TR>  </TBODY>
           my $bFoundMachineResults = "false";
           my $bFailed = "true";
           my $bErrored = "true";

           foreach( @lines )
           {
              if ( $bFoundMachineResults eq "true" )
              {

                 ## check for failures
                 if ( $_ =~ /\-\-Fail\-\-\>0/ ) 
                 {
                    $bFailed = "false";
                 } 

                 ## check for errors
                 if ( $_ =~ /\-\-Errors\-\-\>0/ ) 
                 {
                    $bErrored = "false";
                 } 

              }
            
              if ( $_ =~ /SummaryReportBegin/ )
              {
                 $bFoundMachineResults = "true";
              }
              if ( $_ =~ /SummaryReportEnd/ )
              {
                 last;
              }

          }
          
          if ( $bErrored eq "true" )
          {
             $gBuildFailed = 1;
          }
          if ( $bFailed eq "true" )
          {
             $gBuildFailed = 1;
          }

          # PerformReplace( "Replace \"$gResultRoot/$resultDir/test_output/html/index.html\" \"\\\\index\\.html\" \"/index.html\"" );
          # PerformReplace( "Replace \"$gResultRoot/$resultDir/test_output/html/index.html\" \"\\.\\\\index_delta\\.html\" \"index_delta.html\"" );

       }
     }
     
     Vprint (LOG, "Updating the results homepage to include the newly generated results...(then wait 35 seconds)\n");

     my $installRoot = GetVariable( "DTKDIR" );       
     chdir "$installRoot";   
     MySystem( LOG, "GenerateResults.pl" );
     sleep( 35 );
     Vprint( LOG, "Done." );

     Vprint (LOG, "Processing results to send back to Jenkins...\n");
     my $p4root =  GetVariable( "P4ROOT" );
     foreach $resultDir (@gTestResultDIR)
     {
         MySystem( LOG, "$installRoot\\generatexml.pl \"$p4root\\$resultDir.xml\" $gResultRoot\\$resultDir\\test_output\\html" );
     }

     Vprint (LOG, "Done.\n");

     if ( $gBuildFailed ) 
     {      
         exit 1;
     }    
   }
   else
   {
      Error( "Bad syntax in returnTestResults: '$line'" );
   }
}

#################################################
# Get the version number from logfile
# archived in Jenkins
#
# Param: The log filename
# Param: The Jenkins project name
# Param: The artifact name
# Param: The build number whose artifact is needed
#################################################
sub PerformGetVersionFromLog
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*"(.*)"\s*"(.*)"\s*/ )
   {
      my $file = $1;
      my $jenkinsProject = $2;
      my $findString = $3;
      my $buildNum = $4;
      if ( $buildNum eq "" )
      {
          $buildNum = "lastSuccessfulBuild"
      }
      Vprint( LOG, "build num is $buildNum \n");

      my $url = "$gJenkinsURL/job/$jenkinsProject/$buildNum/artifact";
      my $variable;

      if ($findString =~ /.*\[(.*)\].*/i)
      {
         $variable = $1;
         print( "variable is $variable\n" );
      }
      else
      {
         Error("The variable must be included in [] ");
      }

      $jenkinsProject =~ s/\s/%20/g;

      $findString =~ s/\[$variable\]/\(\.\*\)/;
      Vprint( LOG, "find string is $findString \n");

      getstore( "$url/$file", "$file" ); 
      
      open FILE, "<$file";
      my @lines = <FILE>;
     
      my $found = FALSE;
      for (@lines)
      {       
         if ($_ =~ /$findString/i) 
         {
            if(defined($1))	
            {
               $found = TRUE;
               print("$variable = $1\n");
               AddVariable("$variable",$1);
            }
         }
      }

      close FILE;
      
      if ($found == FALSE)
      {
         Error("Unable to find a value for variable: $variable. Make sure you are matching the correct string.");
      }
   }
   else
   {
      Error( "Bad syntax in GetVersionFromLog: '$line'" );
   }
}

######################################
# Emails the specified recipients if any of the previous installs have failed
#
# Param: string A list of email addresses to notify
# Param: string The subject of the email
# Param: string Path to the install log
#
######################################
sub PerformSendInstallErrors
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
      my $recipient = $1;
      my $subject = $2;

      # send build status email
      if ( $gInstallFailed )
      {
         Vprint (LOG, "emailing PIG install failure to $recipient");
         SendEmail( "$recipient",
                    "$subject",
                    "The attached file contains the complete install log.",
                    "$gLogFile" );

         # reset the build information
         $gInstallFailed = 0;

         Vprint( LOG, "Install failed, exiting" );
         exit 1;
      }
      else
      {
         Vprint( LOG, "Install Generation Successful, no errors to report" );
      }
   }
   else
   {
      Error( "Bad syntax in SendInstallErrors: '$line'" );
   }
}

######################################
# Labels the files in a perforce clientspec with a particular label
#
# Param: string The unique perforce ID to use
# Param: string The label to assign to the tree
######################################
sub PerformLabel
{
   my $line = $_[0];

   if ( not defined $OptVersion )
   {
      Vprint ( LOG, "Not labeling tree - The version number was not incremented." );
      return;
   }

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"(.*)"\s*$/ )
   {
      my $P4id= $1;
      my $label = $2;

      $label =~ s/\[DATE\]/$gDate/g;
      $label =~ s/\[TIME\]/$gTime/g;
      $label =~ s/\[VERSION\]/$gVersion/g;

      my ($root, $client, $server, $user) = GetP4Settings( $P4id );

      ## Label builds in P4
      Vprint (LOG, "labelling build in P4 - $label");
      system ("p4 -c $client -p $server -u $user label -o $label > $root/Label");
      Vprint ( $OptVerbose, "p4 -c $client -p $server -u $user label -o $label > $root/Label" );

      system ("p4 -c $client -p $server -u $user label -i < $root/Label");
      Vprint ( $OptVerbose, "p4 -c $client -p $server -u $user label -i < $root/Label" );

      # add the files from the client to the label
      system ("p4 -c $client -p $server -u $user labelsync -l $label > $root/addLabels.log");
      Vprint ( $OptVerbose, "p4 -c $client -p $server -u $user labelsync -l $label > $root/addLabels.log" );
   }
   else
   {
      Error( "Bad syntax in Label: '$line'" );
   }
}

######################################
# applies the specified expression to every line of the matching files
#
# Param: string Path that may include wildcards which identifies which files to change
# Param: string Perl Regex that will be applied to every line of the matching files
#
# Return: The number of replacements made
######################################
sub PerformReplace
{
   my $line = $_[0];

   my $nReplaceCount = 0;

   if ( $line =~ /^[^\"]+"([^\"]+)"\s*"(.*)"\s*"(.*)"\s*$/ )
   {
      my $filename = $1;
      my $find = $2;
      my $replace = $3;

      ## clean up the string so that when it gets evaluated
      ## the output is what was expected
      $replace =~ s/\\/\\\\/g;
      $replace =~ s/(\$[0-9])/\'.$1.\'/g;
      $replace = "\'$replace\'";

#print "REPLACE: $replace\n";

      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$filename") );

      Vprint( LOG, "Performing replace using: s/$find/$replace/eeg" );

      if ( $OptVerbose )
      {
         my $strFilenames = "Replacing in files:\n";
         foreach ( @matchingFiles )
         {
            $strFilenames .= "$_\n";
         }
         Vprint( $OptVerbose, "$strFilenames" );
      }

      foreach ( @matchingFiles )
      {
         my $match = $_;

         open( FILE, $match ) or
            Error( "Cannot open file: $match for reading: $!" );

         chomp (my @lines = <FILE>);
         close FILE;

         foreach( @lines )
         {
            my $line = $_;
            if ( $line =~ /$find/ )
            {
               Vprint( $OptVerbose, "before: $line");

               $_ =~ s/$find/$replace/eeg;

               Vprint( $OptVerbose, "after : $_\n");

               ## increment count if the lines are different
               if ( $line ne $_ )
               {
                  $nReplaceCount++;
               }
            }
         }

         ## clobber and open the file for writing
         open( FILE, "+>$match" ) or
            Error( "Cannot open file: $match for writing: $!" );

         foreach( @lines)
         {
            print FILE "$_\n";
         }
         close FILE;
      }
   }
   else
   {
      Error( "Bad syntax in Replace: '$line'" );
   }

   return $nReplaceCount;
}

######################################
# creates the specified directory
#
# Param: string Path of the directory to create
#
######################################
sub PerformMkDir
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*$/ )
   {
      my $path = $1;

      my @dirs = split( /\\/, $path );
      my $nDirs = @dirs;
      if ( $nDirs > 1 )
      {
         my $newPath = "";
         foreach( @dirs )
         {
            $newPath .= "$_\\";
            mkdir $newPath;
         }
      }

      mkdir "$path";
      Vprint( LOG, "mkdir $path" );
   }
   else
   {
      Error( "Bad syntax in MkDir: '$line'" );
   }
}

######################################
# executes the specified system command
#
# Param: string The system command to execute
#
######################################
sub PerformSystem
{
   my $line = $_[0];

   my $cmd = "";
   my $log = LOG;

   if ( $line =~ /^[^\"]*"(.*)"\s*"NoLog"\s*/i )
   {
      $cmd = $1;
      $log = 0;
   }
   elsif ( $line =~ /^[^\"]*"(.*)"\s*"Log"\s*/i )
   {
      $cmd = $1;
      $log = 1;
   }
   elsif ( $line =~ /^[^\"]*"(.*)"\s*$/ )
   {
      $cmd = $1;
   }
   else
   {
      Error( "Bad syntax in System: '$line'" );
   }

   Vprint( $log, "$cmd" );

   MySystem( $log, $cmd );
}

######################################
# executes the specified system command as a new process
#
# Param: string The system command to execute
#
######################################
sub PerformExec
{
   my $line = $_[0];

   my $cmd = "";
   my $log = LOG;

   if ( $line =~ /^[^\"]*"(.*)"\s*"NoLog"\s*/i )
   {
      $cmd = $1;
      $log = 0;
   }
   elsif ( $line =~ /^[^\"]*"(.*)"\s*"Log"\s*/i )
   {
      $cmd = $1;
      $log = 1;
   }
   elsif ( $line =~ /^[^\"]*"(.*)"\s*$/ )
   {
      $cmd = $1;
   }
   else
   {
      Error( "Bad syntax in Exec: '$line'" );
   }

   my $pid = MyExec( $log, $cmd );
   Vprint( $log, "$cmd started as process $pid" );

}

######################################
# executes the specified GoogleTest unit test with the specified output.
# Optionally appends a specified suffix to the the testsuite name and 
# classname in the Google Test output file.  This is useful if you want to
# run 32-bit and 64-bit versions of the same tests and have the results
# reported separately on Jenkins
#
# Param: string The full path to the test exe
# Param: string The XML output file for the test
# Param: string Suffix to append to all testnames in the output xml file (optional)
# Param: additional command line params to pass to the test (optional)
#
######################################
sub PerformGoogleTest
{
   my $line = $_[0];
   my $test = "";
   my $output = "";
   my $testsuffix;
   my $quotedoutput = "";
   my $addlparams = "";

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $output = $2;
      $quotedoutput = "\"$output\"";
      $test = "\"\"$1\"  --gtest_output=xml:$quotedoutput\"";
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $output = $2;
      $quotedoutput = "\"$output\"";
      $test = "\"\"$1\"  --gtest_output=xml:$quotedoutput\"";
      $testsuffix = $3;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $output = $2;
      $quotedoutput = "\"$output\"";
      $testsuffix = $3;
      $addlparams = $4;
      $test = "\"\"$1\"  --gtest_output=xml:$quotedoutput $addlparams\"";
   }
   else
   {
      Error( "Bad syntax in GoogleTest: '$line'" );
   }

   ## delete the test output file if it exists from a previous run
   if ( -e $output )
   {
      PerformDelete( "Del $quotedoutput" );
   }

   ## execute the test
   PerformExec( "Exec $test " );
   ## wait for the test result to exist on disk
   PerformWaitForFileExist( "WaitForFileExist $quotedoutput " );

   ## append the specified suffix to the testsuite and classname in the xml output file
   if ( defined $testsuffix )
   {
      PerformReplace ( "Replace $quotedoutput \"testsuite name=\\\"([a-zA-Z]+)\\\"\" \"testsuite name=\"\$1-$testsuffix\"\" " );
      PerformReplace ( "Replace $quotedoutput \"classname=\\\"([a-zA-Z]+)\\\"\" \"classname=\"\$1-$testsuffix\"\" " );
   }
}

######################################
# executes the specified NUnit unit test with the specified output.
#
# Param: string The full path to the nunit test runner exe
# Param: string The full path to the test assembly
# Param: string The XML output file for the test
#
######################################
sub PerformNUnitTest
{
   my $line = $_[0];
   my $test = "";
   my $output = "";
   my $quotedoutput = "";

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $output = $3;
      $quotedoutput = "\"$output\"";
      $test = "\"\"$1\" \"$2\" /xml=$quotedoutput\"";
   }
   else
   {
      Error( "Bad syntax in NUnitTest: '$line'" );
   }

   ## delete the test output file if it exists from a previous run
   if ( -e $output )
   {
      PerformDelete( "Del $quotedoutput" );
   }

   ## execute the test
   PerformExec( "Exec $test " );
   ## wait for the test result to exist on disk
   PerformWaitForFileExist( "WaitForFileExist $quotedoutput " );
}

######################################
# copies a directory from one location to another
#
# Param: string Path to the source
# Param: string Path to the destination
#
######################################
sub PerformXCopy
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
      my $src = $1;
      my $dest = $2;

      $dest =~ s/\//\\/g;

      if ( $dest =~ /\\$/ )
      {
         ## the dest is a directory (ends with \ )
         unless ( -d "$dest" )
         {
            ## directory doesn't exist yet, make sure it exists so that we can copy into it
            PerformMkDir( " \"$dest\" ")
         }
      }

      Vprint( LOG, "xcopy /R/S/Y \"$src\" \"$dest\"" );
      MySystem( $OptVerbose, "xcopy /R/S/Y \"$src\" \"$dest\"" );
   }
   else
   {
      Error( "Bad syntax in XCopy: '$line'" );
   }
}

######################################
# copies a file from one location to another
#
# Param: string Path to the source file
# Param: string Path to the destination
#
######################################
sub PerformCopy
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"(.*)"\s*"(.*)"\s*$/ )
   {
      my $src = $1;
      my $dest = $2;

      $dest =~ s/\//\\/g;

      if ( $dest =~ /\\$/ )
      {
         ## the dest is a directory (ends with \ )
         unless ( -d "$dest" )
         {
            ## directory doesn't exist yet, make sure it exists so that we can copy into it
            PerformMkDir( " \"$dest\" ")
         }
      }

      Vprint( LOG, "copy \"$src\" \"$dest\"" );

      MySystem( $OptVerbose, "copy \"$src\" \"$dest\"" );
   }
   else
   {
      Error( "Bad syntax in Copy: '$line'" );
   }
}

######################################
# builds a configuration of a project within a solution file
#
# Param: string Path to the solution file
# Param: string Name of the project to build
# Param: string Name of the build configuration to run
#
######################################
sub PerformBuildSln
{
   my $line = $_[0];

   my $solutionFile;
   my $project;
   my $action;
   my $configuration;
   my $addedDefines;

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $solutionFile = $1;
      $project = $2;
      $action = $3;
      $configuration = $4;
      $addedDefines = "";
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $solutionFile = $1;
      $project = $2;
      $action = $3;
      $configuration = $4;
      $addedDefines = $5;
   }
   else
   {
      Error( "Bad syntax in BuildSln: '$line'" );
   }

   if ( $solutionFile =~ /^\s*$/ )
   {
      Error( "Solution File not specifed in BuildSln" );
   }

   if ( $configuration =~ /^\s*$/ )
   {
      Error ( "configuration not specified in BuildSln" );
   }

   my $msg = "";
   my $cmd = "devenv ";
   my $vsVars = "";
   my $stoponerror = "FALSE";

   $msg .= "$solutionFile";
   $cmd .= "\"$solutionFile\"";

   if ( -e $solutionFile )
   {
      if ( $addedDefines !~ /^\s*$/ )
      {
         ## use this for now, it may have problems with projects that already contain PreprocessorDefines
         AddDefinesToSln( $solutionFile, $project, $configuration, $addedDefines );
      }

      open( FILE, "$solutionFile" );

      chomp (my @lines = <FILE>);
      close FILE;
      
      my $vs2003Path = "C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Common7\\Tools\\vsvars32.bat";
      my $vs2005Path = "C:\\Program Files\\Microsoft Visual Studio 8\\Common7\\Tools\\vsvars32.bat";
      my $vs2008Path = "C:\\Program Files\\Microsoft Visual Studio 9.0\\Common7\\Tools\\vsvars32.bat";
      my $vs2010Path = "C:\\Program Files\\Microsoft Visual Studio 10.0\\Common7\\Tools\\vsvars32.bat";
      my $vs2012Path = "C:\\Program Files\\Microsoft Visual Studio 11.0\\Common7\\Tools\\vsvars32.bat";
      my $vs2013Path = "C:\\Program Files\\Microsoft Visual Studio 12.0\\Common7\\Tools\\vsvars32.bat";
      my $vs2015Path = "C:\\Program Files\\Microsoft Visual Studio 14.0\\Common7\\Tools\\vsvars32.bat";

      foreach( @lines)
      {
         if ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s8\.00/ )
         {
            ## Visual Studio 2003
            $vsVars = $vs2003Path;
         }
         elsif ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s9\.00/ )
         {
            ## Visual Studio 2005
            $vsVars = $vs2005Path;
         }
         elsif ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s10\.00/ )
         {
            ## Visual Studio 2008
            $vsVars = $vs2008Path;
         }
         elsif ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s11\.00/ )
         {
            ## Visual Studio 2010
            $vsVars = $vs2010Path;
         }
	 #elsif ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s12\.00/ )
	 #{
	 #   ## This can be either VS2012 or VS2013, assume it's the older one:
	 #   ## Visual Studio 2012
	 #   ## $vsVars = $vs2012Path;
	 #}
        elsif ( $_ =~ /#\sVisual\sStudio\s2012/ )
        {
            ## Visual Studio 2012
            $vsVars = $vs2012Path;
        }
        elsif ( $_ =~ /#\sVisual\sStudio\s2013/ )
        {            
            ## Visual Studio 2013
            $vsVars = $vs2013Path;
         }
        elsif ( $_ =~ /#\sVisual\sStudio\s14/ )
        {            
            ## Visual Studio 2015
            $vsVars = $vs2015Path;
         }
      }

      # if building on an x64 system - Visual Studio is installed in a different location
      unless (-e $vsVars)
      {
         $vsVars =~ s/Program\sFiles/Program Files (x86)/;
      }

      if( $vsVars eq "" )
      {
         UndoAddDefinesToSln();
         Error( "Cannot determine Visual Studio version of: $solutionFile" );
      }
   }
   else
   {
      Error( "File does not exist: $solutionFile" );
   }

   if ( $project !~ /^\s*$/ )
   {
      $msg .= "-$project";
      $cmd .= " /Project \"$project\"";
   }

   if ( $action =~ /^Build$/i )
   {
      $msg = "Building $msg";
      $cmd .= " /Build";
   }
   elsif ( $action =~ /^Rebuild$/i )
   {
      $msg = "Rebuilding $msg";
      $cmd .= " /Rebuild";
   }
   elsif ( $action =~ /^Clean$/i )
   {
      $msg = "Cleaning $msg";
      $cmd .= " /Clean";
   }
   elsif ( $action =~ /^BuildandTest$/i )
   {
      $msg = "Building $msg";
      $cmd .= " /Build";
      $stoponerror = "TRUE";
   }
   else
   {
      UndoAddDefinesToSln();
      Error ( "Action not specified in BuildSln" );
   }

   $msg .= "-$configuration";
   $cmd .= " \"$configuration\"";

   Vprint (LOG, $msg);

   MySystem ( LOG, "\"$vsVars\" & $cmd", "true");
   my $result = $? >> 8;
   Vprint (LOG, "$msg returned: $result\n");
   if ( $result )
   {
      $gBuildFailed += $result;
      $gBuildFailedMessage .= "$msg failed. Status = $?\n";
      if  ($stoponerror eq "TRUE")
      {
         Error ($gBuildFailedMessage);
      }
   }

   UndoAddDefinesToSln();
}


#######################################
# seatches a file for any refrence to version numbering so that it will be
# insync with version passed in by $newVersionString.
# Used by PerformVersionUpdateOnTool and PerformBrowserUpdate
# Created for Handheld Imageon SDK
#
# Param: string Name of file to search for version strings
# Param: string Version as a string to replace. Format xx.xx.xx.xx or xx,xx,xx,xx
#
#######################################

sub PerformVersionSearchOnRcFile
{
    my $fileName = shift;
    my $newVersionString = shift;
    my $line;
    my $tempFileName = "DeleteMeFromRelease.txt";
    open(ORIGNIALFILE,"<",$fileName) or
            Error( "PerformVersionSearchOnRcFile::Cannot open file to change version info on Browser or tool:$fileName $!" );
    open(TEMPFILE,">",$tempFileName ) or
            Error( "PerformVersionSearchOnRcFile::Cannot open temporary file: $tempFileName. $!" );

    while ($line = <ORIGNIALFILE>)
    {
        if ($line =~ "FILEVERSION")
        {
            print TEMPFILE  " FILEVERSION $newVersionString\n";
        }
        elsif ($line =~ "PRODUCTVERSION ")
        {
            print TEMPFILE  " PRODUCTVERSION  $newVersionString\n";
        }
        elsif ($line =~ "VALUE \"FileVersion\"")
        {
            print TEMPFILE  "            VALUE \"FileVersion\", \"$newVersionString\"\n";
        }
        elsif ($line =~ "VALUE \"ProductVersion\"")
        {
            print TEMPFILE  "            VALUE \"ProductVersion\", \"$newVersionString\"\n";
        }
        elsif ($line =~ "AssemblyFileVersionAttribute")
        {
            print TEMPFILE  "[assembly: AssemblyFileVersionAttribute(\"$newVersionString\")]\n";
        }
        else
        {
            print TEMPFILE $line;
        }
     }

     close(ORIGNIALFILE);
     close(TEMPFILE);

     open (SAMEFILE,">", $fileName) or
            Error( "PerformVersionSearchOnRcFile::Cannot open file to write change version info on Browser or tool:$fileName $!" );
     open(TEMPFILE,"<",$tempFileName) or
            Error( "PerformVersionSearchOnRcFile::Cannot open temporary file for readback: $tempFileName. $!" );

     while ($line = <TEMPFILE>)
     {
       print SAMEFILE $line;
     }

     close(SAMEFILE);
     close(TEMPFILE);
     MySystem( $OptVerbose,"del $tempFileName", "true");
}
#######################################
# updates version information on a tool and then builds it with PerfromBuilSln
# if using, notice how $buildline is created and expand it if necessary.
# Created for Handheld Imageon SDK
#
# Param: string path is the directory the solution is located
# Param: string tool is the format its in
# Param: string build either "Release" or "Debug" for PerformBuildSln
#
#######################################
sub PerformVersionUpdateOnTool
{
   my $line = $_[0];
   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
        my $path = $1;
        my $tool = $2;
        my $version;
        if ( (defined(GetVariable("MAJOR"))) && ( defined(GetVariable("MINOR")) )&& ( defined(GetVariable("BUILD")) )&& ( defined(GetVariable("UPDATE")) ))
        {
            my $major = GetVariable("MAJOR");
            my $minor = GetVariable("MINOR");
            my $update = GetVariable("UPDATE");
            my $build = GetVariable("BUILD");
            $version = sprintf "$major,$minor,$update,$build";
        }
        else
        {
            printf "Warning: Version is hardcoded with date at end.\n";
            $version = sprintf "1,0,%02s%02s,%02s%02s", $year-100, $mon+1, $mday, $hour;
        }
        my $build = $3;

        my $toolFolder = "$path\\$tool";
        my $rcFile = "$path\\$tool\\$tool\.rc";
        my $deleteline = "\"$toolFolder\\$build\\\"";

        chdir $toolFolder;
        MySystem( $OptVerbose,"attrib -R *.*", "true");
        PerformVersionSearchOnRcFile($rcFile, $version);
        my $buildline = "\"$path\\$tool\\$tool\.sln\" \"\"  \"Build\" \"$build\"   ";
        PerformBuildSln($buildline);
        MySystem( $OptVerbose, "rd /S/Q $deleteline" );
   }
}
#######################################
# updates version information on Broswer (C#) and then builds it with PerfromBuilSln
# if using, notice how hardcoded it is and expand it if necessary.
# Created for Handheld Imageon SDK
#
# Param: string path is the directory the solution is located
# Param: string tool is the format its in
# Param: string build either "Release" or "Debug" for PerformBuildSln
#
#######################################
sub PerformBrowserUpdate
{
    my $line = $_[0];
    if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
    {
        my $path = $1;
        my $tool = $2;

        my $version; # = sprintf "1.0.%02s%02s.%02s%02s", $year-100, $mon+1, $mday, $hour;
        if ( (defined(GetVariable("MAJOR"))) && ( defined(GetVariable("MINOR")) )&& ( defined(GetVariable("BUILD")) )&& ( defined(GetVariable("UPDATE")) ))
        {
            my $major = GetVariable("MAJOR");
            my $minor = GetVariable("MINOR");
            my $update = GetVariable("UPDATE");
            my $build = GetVariable("BUILD");
            $version = sprintf "$major.$minor.$update.$build";
        }
        else
        {
            printf "Warning: Version is hardcoded with date at end.\n";
            $version = sprintf "1.0.%02s%02s.%02s%02s", $year-100, $mon+1, $mday, $hour;
        }

        my $build = $3;

        my $toolFolder = "$path\\Tools\\Internal\\$tool";
        # NOTE  path changed here!
        # my $rcFile = "$path\\Tools\\Internal\\$tool\\AssemblyInfo.cs";
        my $rcFile = "$path\\Utilites\\Internal\\$tool\\AssemblyInfo.cs";
        chdir "$path\\Browser";
        MySystem( $OptVerbose,"attrib -R *.*", "true");
        chdir $toolFolder;
        MySystem( $OptVerbose,"attrib -R *.*", "true");
        PerformVersionSearchOnRcFile($rcFile, $version);
        #my $buildline = "\"$path\\Tools\\Internal\\$tool\\$tool\.sln\" \"\"  \"Build\" \"$build\"   ";
        my $buildline = "\"$path\\Utilities\\Internal\\$tool\\$tool\.sln\" \"\"  \"Build\" \"$build\"   ";
        PerformBuildSln($buildline);
   }
}
#######################################
# makes sln file with MakeProjectFiles.bat and then calls PerformBuildSln
# Created for Handheld Imageon SDK
#
# Param: string Path to the solution file
# Param: string Name of the project to build
# Param: string Name of the action
# Param: string Name of the build configuration to run#
#
#######################################
sub PerformBuildMacro
{
   my $line = $_[0];
   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      my $dir = $1;
      my $buildline = "\"$dir\\Win32\\$2\" \"\" \"$3\" \"$4\"";
      my $deleteline = "\"$dir\\Win32\\Release\\\"";

      chdir $dir;
      MySystem( $OptVerbose,"attrib -R *.*", "true");
      MySystem( $OptVerbose,"MakeProjectFiles.bat", "true");
      PerformBuildSln($buildline);

      MySystem( $OptVerbose, "rd /S/Q $deleteline" );
      MySystem( $OptVerbose, "del /S/Q MakeProjectFiles.bat");
    }
}
#######################################
# deletes all excess files after a build.
# Created for Handheld Imageon SDK
#
# Param: string Path to the Dir
#
#######################################
sub PerformTutorialClean
{
   my $line = $_[0];
   if ( $line =~  /^[^\"]*"([^\"]*)"\s*$/  )
   {
      my $dir = $1;
      my $releaseDir = "\"$dir\\Win32\\Release\"";
      chdir $releaseDir;
      MySystem( $OptVerbose, "del /S/Q $releaseDir\\*.htm");
      MySystem( $OptVerbose, "del /S/Q $releaseDir\\*.manifest");
      MySystem( $OptVerbose, "del /S/Q $releaseDir\\*.pdb");
      MySystem( $OptVerbose, "del /S/Q $releaseDir\\*.obj");
      MySystem( $OptVerbose, "del /S/Q $releaseDir\\*.dep");
      MySystem( $OptVerbose, "del /S/Q $releaseDir\\*.idb");
   }
}
######################################
# Makes a file writeable (not read-only)
#
# Param: string A path to a file which should be made writeable
#
######################################
sub PerformMakeWriteable
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      my $path = $1;

      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$path") );

      foreach( @matchingFiles )
      {
         my $file = $_;
         if ( -e "$file" )
         {
            if ( -w "$file" )
            {
               ## file is already writeable
            }
            else
            {
               unless( chmod( 0666, $file ) )
               {
                  Vprint( LOG, "Could not chmod '$file': $!" );
               }
            }
         }
         else
         {
            Error( "Could not make file writeable: '$file' does not exist" );
         }
      }
   }
   else
   {
      Error( "Bad syntax in MakeWriteable: '$line'" );
   }
}

######################################
# Makes a file read-only (not writeable)
#
# Param: string A path to a file which should be made read-only
#
######################################
sub PerformMakeReadOnly
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      my $path = $1;

      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$path") );

      foreach( @matchingFiles )
      {
         my $file = $_;
         if ( -e "$file" )
         {
            if ( -w "$file" )
            {
               ## file is writeable
               unless( chmod( 0444, $file ) )
               {
                  Vprint( LOG, "Could not chmod '$file': $!" );
               }
            }
         }
         else
         {
            Error( "Could not make file read-only: '$file' does not exist" );
         }
      }
   }
   else
   {
      Error( "Bad syntax in MakeReadOnly: '$line'" );
   }
}


######################################
# Removes the specified files or directories from the computer
#
# Param: string A path which may contain wildcards that specify the directory or file to delete
#
######################################
sub PerformDelete
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      my $path = $1;

      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$path") );

      foreach( @matchingFiles )
      {
         my $file = $_;
         if ( -e "$file" )
         {
            if ( -d "$file" )
            {
               if( MySystem( $OptVerbose, "rmdir /S /Q \"$file\"", TRUE) == FALSE )
               {
                  Warning( "System Command: 'rmdir /S /Q \"$file\"' did not succeed, but the build will continue. The output may use previously built files." );
               }
            }
            else
            {
               Vprint ( LOG, "del \"$file\"" );
               unlink( "$file" );
            }
         }
      }

      my $nMatches = @matchingFiles;

      if ( $nMatches == 0 )
      {
         Vprint ( LOG, "$path Could not be deleted: it does not exist" );
      }
   }
   else
   {
      Error( "Bad syntax in Del: '$line'" );
   }
}

######################################
# Create a clientSpec on the specified server, with the specified root directory
#
# Param: Unique name to assign to these perforce settings
# Param: path to assign as the root of the clientspec
# Param: name of the client to create
# Param: name of the server to create the client on
# Param: name of the user that is creating the client
#
######################################
my @lstCreatedClients;
sub PerformMakeClient
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      my $P4id = $1;
      my $P4Root = $2;
      my $P4Client = $3;
      my $P4Server = $4;
      my $P4User = $5;

      if ( $P4Client eq "ma_3dargbld" )
      {
         Error( "Cannot delete or create ma_3dargdbld - it is necessary for nightly builds" );
      }

      Vprint ( LOG, "Creating clientspec $P4Client on $P4Server" );

      ##make the root now so that we can save the clientspec file in it (PerformMkDir will recursively create the directories)
      PerformMkDir( " \"$P4Root\"");

      ## check to see if the specified client already exists
      my @allClients = `p4 -p $P4Server clients`;
      foreach ( @allClients )
      {
         ## if Client line is related to the desired client
         if ( $_ =~ /^Client $P4Client/ )
         {
            ## Check to see if it is owned by the desired user
            if ( $_ =~ /\'Created by $P4User\. \'$/ )
            {
               ## if so, delete it so that it can be recreated
               Vprint( LOG, "User OWNS client: $_" );
               Vprint( LOG, "Recreating Client $P4Client" );
               MySystem( $OptVerbose, "p4 -p $P4Server -u $P4User client -d -f $P4Client" );
            }
            else
            {
               Vprint( LOG, "User DOES NOT own client: $_" );
               Error( "Cannot Recreate client $P4Client because it is not owned by $P4User" );
            }
         }
      }

      ## at this point, three possible scenarios:
      ## 1) the client either never existed, so we can create it
      ## 2) the client was owned by this user, and was deleted, so we can recreate it
      ## 3) the client was owned by a different user, and the error above stopped everything (and we don't actually get to this line).
      ## because the of the file redirection, this CANNOT use the MySystem call!
      system( "p4 -c $P4Client -p $P4Server -u $P4User client -o > \"$P4Root\\clientspec\"" );

      if ( $? )
      {
         Error( "Could not create client $P4Client on $P4Server" );
      }

      open( CLIENTSPEC, "$P4Root\\clientspec" ) or Error("Could not open '$P4Root\\clientspec' for reading" );
      chomp( my @lines = <CLIENTSPEC> );
      close CLIENTSPEC;

      open( CLIENTSPEC, "+>$P4Root\\clientspec" ) or Error("Could not open '$P4Root\\clientspec' for reading" );

      ## From the initial clientspec, we don't want to include ANYTHING related to the default view,
      ## so we will not write out anything after the line containing "View:"
      my $bPassedView = "false";
      foreach( @lines )
      {
         if ( $_ =~ /^Host:.*$/ )
         {
            $_ = "Host:\t";
         }
         elsif ( $_ =~ /^Root:.*$/ )
         {
            $_ = "Root:\t$P4Root";
         }
         elsif ( $_ =~ /^View:$/ )
         {
            $bPassedView = "true";
         }
         elsif ( $bPassedView eq "true" )
         {
            $_ = "\t";
         }

         print CLIENTSPEC "$_\n";
      }
      close CLIENTSPEC;

      MySystem( $OptVerbose, "p4 -p $P4Server -u $P4User client -i < \"$P4Root\\clientspec\"" );

      if ( $? )
      {
         Error( "Could not create client $P4Client on $P4Server" );
      }

      push( @lstCreatedClients, $P4Client );
      AddP4Settings( $P4id, $P4Root, $P4Client, $P4Server, $P4User );
   }
   else
   {
      Error( "Bad syntax in MakeClient: '$line'" );
   }
}

######################################
# Adds the specified view to the client
#
# Param: Unique name assigned to the perforce settings to use
# Param: Path in the depot that should be added to the clientspec
#
######################################
sub PerformUpdateClient
{
   my $line = $_[0];

   my $P4id;
   my $depotPath;
   my $localPath;

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $P4id = $1;
      $depotPath = $2;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $P4id = $1;
      $depotPath = $2;
      $localPath = $3;
   }
   else
   {
      Error( "Bad syntax in UpdateClient: '$line'" );
   }

   my ( $root, $client, $server, $user ) = GetP4Settings( $P4id );

   my $bAllowedToEdit = "false";
   foreach( @lstCreatedClients )
   {
      if ( "$_" eq "$client" )
      {
         $bAllowedToEdit = "true";
      }
   }

   if ( $bAllowedToEdit eq "false" )
   {
      Error( "Cannot edit client '$client' because it was not made by this build configuration" );
   }

   ## because the of the file redirection, this CANNOT use the MySystem call!
   Vprint ( LOG, "Updating clientspec $client on $server" );
   system( "p4 -p $server -u $user client -o -t $client $client > \"$root\\clientspec\"" );
   if ( $? )
   {
      unlink "$root\\clientspec";
      Error( "Could not update client $client on $server" );
   }

   open(EDIT, ">>$root\\clientspec")
        or Error("Cannot open file: $root\\clientspec for appending: $!");

   ## check if the edit already contains a local path
   ## this looks for characters followed by a space followed by //
   if ( defined $localPath )
   {
      ## this edit specifies the entire update to make
      print EDIT "View:\t\"$depotPath\" \"$localPath\"";
   }
   else
   {
      ## this only contains the depot side of the update, so replace the first part of the path with the local root
      $localPath = $depotPath;

      ## replace //any.non-forwardslash_chars/ (with or without leading +,-) with //$client/
      ## this eliminates the need for users to type the path in twice
      ## and the chance for them to mess up the format
      $localPath =~ s/^[+-]?\/\/[^\/]+\//\/\/$client\//;

      print EDIT "View:\t\"$depotPath\" \"$localPath\"";

   }

   close EDIT;

   MySystem( $OptVerbose, "p4 -c $client -p $server -u $user client -i < \"$root\\clientspec\"" );

   if ( $? )
   {
      Error( "Could not edit client $client on $server" );
   }
}

######################################
# Deletes a perforce clientspec provided that the client was created by the same build configuration
#
# Param: Unique name assigned to these perforce settings that indicates which server and client to delete
#
######################################
# Only allows deletion of clients created in this build configuration
sub PerformDeleteClient
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      my $P4id = $1;

      my ( $root, $client, $server, $user ) = GetP4Settings( $P4id );

      my $bAllowedToDelete = "false";
      foreach( @lstCreatedClients )
      {
         if ( "$_" eq "$client" )
         {
            $bAllowedToDelete = "true";
         }
      }

      if ( $bAllowedToDelete eq "true" )
      {
         Vprint ( LOG, "Deleting clientspec $client from $server" );
         Vprint ( LOG, "p4 -p $server -u $user client -d $client" );
         my $output = `p4 -p $server -u $user client -d $client`;
         if( $output =~ /Client.*has files opened/i )
         {
            Vprint( LOG, "   Client has files opened, attempting to revert before deleting client" );
            MySystem( $OptVerbose, "p4 -c $client -p $server -u $user revert -a" );

            Vprint( LOG, "   Trying again to delete the client" );
            $output = `p4 -p $server -u $user client -d $client`
         }

         if( $output !~ /Client.*has files opened/i )
         {
            RemoveP4Settings( $P4id );
         }
      }
      else
      {
         Error( "Cannot delete client '$client' because it was not made by this build configuration" );
      }
   }
   else
   {
      Error( "Bad syntax in DeleteClient: '$line'" );
   }
}

######################################
# Updates an entire clientspec from a perforce server (doesn't force sync)
#
# Param: string \sa PerformSync
#
######################################
sub PerformUpdate
{
   my $line = $_[0];
   PerformSync( $line, "false" );
}

######################################
# Sync an entire clientspec from a perforce server
#
# Param: string Unique name to assign to these perforce settings
# Param: string path to the root of the local perforce tree
# Param: string name of the client to sync the files as
# Param: string name of the server that holds the perforce tree
# Param: string name of the user that is syncing the files
#
######################################
sub PerformSync
{
   my $line = $_[0];

   ## make force sync default to true
   my $bForceSync = "true";
   if ( defined $_[1] )
   {
      $bForceSync = $_[1];
      Vprint( LOG, "Forcing sync: $bForceSync" );
   }

   my $P4id;
   my $P4Root;
   my $P4Client;
   my $P4Server;
   my $P4User;
   my $P4Revision = "";

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*$/ )
   {
      $P4id = $1;

      ($P4Root, $P4Client, $P4Server, $P4User) = GetP4Settings( $P4id );
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $P4id = $1;
      $P4Revision = $2;

      ($P4Root, $P4Client, $P4Server, $P4User) = GetP4Settings( $P4id );
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $P4id = $1;
      $P4Root = $2;
      $P4Client = $3;
      $P4Server = $4;
      $P4User = $5;

      AddP4Settings( $P4id, $P4Root, $P4Client, $P4Server, $P4User );
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
      $P4id = $1;
      $P4Root = $2;
      $P4Client = $3;
      $P4Server = $4;
      $P4User = $5;
      $P4Revision = $6;

      AddP4Settings( $P4id, $P4Root, $P4Client, $P4Server, $P4User );
   }
   else
   {
      Error( "Bad syntax in Sync: '$line'" );
   }

   my $cmd = "p4 -c $P4Client -p $P4Server -u $P4User sync";
   my $text = "Syncing entire tree using client $P4Client";

   ## check if we should force sync or not
   if ( $bForceSync eq "true" )
   {
      $cmd .= " -f";
      $text = "Force $text";
   }

   my $label = $OptLabel;
   if ( $P4Revision )
   {
      $label = $P4Revision;
   }

   ## check if we should sync to a specified label
   if ( $label )
   {
      $cmd .= " \@$label";
      $text .= " to label $label";
   }

   # $P4Root is the default sync directory. Note. This must match the root of the p4 client

   ## we used to delete the root directory here to make sure it is empty,
   ## but with multiple perforce servers, they could sync to the same directory
   ## and we don't want to blow away an earlier sync

   ## Sync the source files from P4
   Vprint (LOG, "$text");
   MySystem ( $OptVerbose , "$cmd");
}

######################################
# Adds files to the default changelist matching the specified pattern
#
# Param: string The Perforce ID to add the files to
# Param: string A file that will be added to the default perforce changelist
#
######################################
sub PerformP4Add
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"(.*)"\s*$/ )
   {
      my $P4id = $1;
      my $files = $2;

      my ( $root, $client, $server, $user ) = GetP4Settings( $P4id );

      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$files") );

      foreach ( @matchingFiles )
      {
         my $match = $_;

         if ( -d $match )
         {
            ## recurse adding files in this directory
            PerformP4Add( "P4Add \"$P4id\" \"$match\\*.*\"" );

            ## recurse again adding directories
            PerformP4Add( "P4Add \"$P4id\" \"$match\\*\"" );
         }
         elsif ( -e $match )
         {
            ## add the file if it exists
            Vprint( LOG, "Adding to Perforce: $match" );

            MySystem( $OptVerbose, "p4 -c $client -p $server -u $user add $match" );
         }
         else
         {
            Error( "Cannot add file to Perforce: file does not exist: $match" );
         }
      }
   }
   else
   {
      Error( "Bad syntax in P4Add: '$line'" );
   }
}

######################################
# Checks out the specified file
#
# Param: string The Perforce ID to check out the file using
# Param: string A file that will be checked out of the perforce tree
#
######################################
sub PerformCheckOut
{
   my $line = $_[0];

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"(.*)"\s*$/ )
   {
      my $P4id = $1;
      my $versionHead = "$2";

      my ( $root, $client, $server, $user ) = GetP4Settings( $P4id );

      # check the file out for edit
      Vprint ( LOG, "checking out $versionHead" );
      system ("p4 -c $client -p $server -u $user edit \"$versionHead\"");
   }
   else
   {
      Error( "Bad syntax in CheckOut: '$line'" );
   }
}

######################################
# Checks in all the currently checked out files
#
# Param: string The Perforce ID to check in the files with
# Param: string a description that will be used in the changelist
#
######################################
sub PerformCheckIn
{
   my $line = $_[0];

   my $P4id;
   my $description;
   my $file;

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"(.*)"\s*"([^\"]+)"\s*$/ )
   {
      $P4id = $1;
      $description = $2;
      $file = $3;
   }
   elsif ( $line =~ /^[^\"]*"([^\"]*)"\s*"(.*)"\s*$/ )
   {
      $P4id = $1;
      $description = $2;
      $file = "";
   }
   else
   {
      Error( "Bad syntax in CheckIn: '$line'" );
   }

      my ( $root, $client, $server, $user ) = GetP4Settings( $P4id );

      ## first revert any unchanged files
      Vprint( LOG, "Reverting any unchanged files prior to CheckIn" );
      MySystem( $OptVerbose, "p4 -c $client -p $server -u $user revert -a" );

      ## check to see if there are files to submit
      my @openedFiles = `p4 -p $server -u $user opened -C $client`;
      my $bCheckInFiles = "false";

      my $depotPath = "";
      if ( $file eq "" )
      {
         my $nOpenedFiles = @openedFiles;
         if ( $nOpenedFiles > 0 )
         {
            $bCheckInFiles = "true";
         }
      }
      else
      {
         ## a file was specified, so check for that specific file

         ## make the file a valid regex
         my $tmpFile = $file;
         my $tmpRoot = $root;
         $tmpRoot =~ s/\\/\\\\/g;
         if ( $file =~ /^$tmpRoot(.*)/io )
         {
            $tmpFile = $1;
         }
         $tmpFile =~ s/\\/\//g;
         #print"\n\n\nSEARCHING FOR: $tmpFile\n";
         foreach ( @openedFiles )
         {
            my $oFile = $_;
            #print "   '$oFile'\n";
            if ( $oFile =~ /(.*)$tmpFile/ )
            {
               $depotPath = "$1$tmpFile";
               #print "   MATCHED!!!  depot root is '$depotPath'\n\n\n";
               $bCheckInFiles = "true";
            }
         }
      }

      if ( $bCheckInFiles eq "false" )
      {
         Vprint( LOG, "There are no files to CheckIn for line: $line" );
      }
      else
      {
         $description =~ s/\[DATE\]/$gDate/g;
         $description =~ s/\[TIME\]/$gTime/g;
         $description =~ s/\[VERSION\]/$gVersion/g;

         Vprint ( $OptVerbose, "Creating changelist $root/changelist");
         open(CHANGE, "> $root/changelist")
            or Error("Cannot open file: $root/changelist for writing: $!");

         my $bWriteNextLine = "true";
         my @changelist = `p4 -c $client -p $server -u $user change -o`;
         chomp(@changelist);
         foreach (@changelist)
         {
            if (/^.*enter description here/)
            {
               my @descriptionLines = split( /\\n/, $description );

               foreach( @descriptionLines )
               {
                  print CHANGE "\t$_\n";
               }
            }
            elsif ( /^Files:/ )
            {
               print CHANGE "Files:\n";

               if ( $file ne "" )
               {
                  $bWriteNextLine = "false";
                  print CHANGE "\t$depotPath\n";
               }
            }
            elsif ( $bWriteNextLine eq "true" )
            {
               print CHANGE "$_\n";
            }
         }
         close CHANGE;
         Vprint (LOG, "submitting changelist with description: '$description'");

         MySystem ($OptVerbose, "p4 -c $client -p $server -u $user submit -i < $root/changelist");
      }
}

###############################
#  P4Have "client" "server" "user" "file"
#
# Param: string client name
# Param: string server name
# Param: string user name
# Param: string file name
#
# Return: "true" if file exists at this perforce server under the specified client; "false" otherwise
#
###############################
sub P4Have
{
   my $client = $_[0];
   my $server = $_[1];
   my $user = $_[2];
   my $file = $_[3];

   my $bHasFile = "true";

   my $output = `p4 -c $client -p $server -u $user have \"$file\" 2>&1`;

   if ( $output =~ /file\(s\) not on client./ or
        $output =~ /Path.*is not under client/)
   {
      $bHasFile = "false";
   }

   return $bHasFile;
}

######################################
#
# The following functions are just helper functions
# and do not directly perform any of the actions
# that are available in the configuration file
#
######################################

######################################
# print message and exit on error.
# If $OptMail specfied then also email error message
#
# Param: string message to display
#
######################################
sub Error
{
    my ($errorMsg) = @_;

    Vprint( LOG, "ERROR: $errorMsg\n" );

    if (defined $OptMail && defined $gLogFile)
    {
        Vprint (LOG, "emailing error message to $OptMail : $errorMsg");
        SendEmail( "$OptMail", "Automated Build Error", "The attached configuration file and log file resulted in the following".
        " errors:\n\n\t$errorMsg", "$OptConfig, $gLogFile" );
    }


    exit 1;
}

######################################
# print message on warning
# If $OptMail specfied then also email warning message
#
# Param: string message to display
#
######################################
sub Warning
{
    my ($warnMsg) = @_;

    Vprint( LOG, "WARNING: $warnMsg\n" );

    if (defined $OptMail)
    {
        Vprint (LOG, "emailing warning message to $OptMail");
        SendEmail( "$OptMail", "Automated Build Warning", "The attached configuration file resulted in the following".
        " warning:\n\n\t$warnMsg", "$OptConfig" );
    }
    else
    {
        Vprint (LOG, "emailing warning message to dl.DevToolsBOS\@amd.com");
        SendEmail( "dl.DevToolsBOS\@amd.com", "Automated Build Warning", "The attached configuration file resulted in the following".
        " warning and there is no email address associated with it:\n\n\t$warnMsg", "$OptConfig" );
    }
}


######################################
# Sends an email
#
# Param: string email addresses to send email to
# Param: string subject line
# Param: string message body
# Param: string path to a file to attach to the email
#
######################################
sub SendEmail
{
   my ($strRecipient, $strSubject, $strMsg, $strAttachment) = @_;

   if ( defined $OptMail )
   {
      Vprint( LOG, "Sending Email using '$gEmailServer' from '$gEmailSender'");
      Vprint( LOG, "Email To     : $strRecipient");
      Vprint( LOG, "Email Subject: $strSubject");
      if ( defined $strAttachment )
      {
          Vprint( LOG, "Email Attach : $strAttachment");
      }
      Vprint( LOG, "Email Message: $strMsg");


      my $sender = new Mail::Sender{ smtp => $gEmailServer, from => $gEmailSender };

      my $result;
      if ( defined $strAttachment )
      {
         $result = $sender->MailFile({      to => $strRecipient,
                                       subject => "$strSubject",
                                           msg => "$strMsg",
                                          file => "$strAttachment" } );
      }
      else
      {
         $result = $sender->MailMsg({      to => $strRecipient,
                                      subject => "$strSubject",
                                          msg => "$strMsg" } );
      }

      if( $result < 0 )
      {
         Vprint( LOG, "Failed to send email. Error: $Mail::Sender::Error" );
         Vprint( LOG, "Trying again without attachements..." );
         $result = $sender->MailMsg({      to => $strRecipient,
                                      subject => "$strSubject",
                                          msg => "$strMsg" } );
         if ( $result < 0 )
         {
            Vprint( LOG, "Failed second attempt to send email. No more attempts will be made. Error: $Mail::Sender::Error" );
         }
      }
   }
}

######################################
# replace standard "exec" command with a custom version.
#
# Param: string command to execute
#
# Return: Pid of the launched application
######################################
sub MyExec
{
   my ( $log, $cmd ) = @_;

   Vprint( $log, "Launching: $cmd" );

   my $pid = fork;

   if ( not defined $pid )
   {
      Error( "Could not fork before launching: $cmd" );
   }
   elsif ( $pid == 0 )
   {
      ## child process
      exec( $cmd );
   }
   else
   {
      ## parent process
      ## $pid was set to the childs pid, so there is nothing special to do.
   }

   return $pid;
}

######################################
# replace standard "system" command with a custom version.
#
# Param: boolean indicates whether to redirect output of system call to the log file
# Param: string command to execute
#
# Return: boolean indicating if system call was successful
#
######################################
sub MySystem
{
    my ($log, $command, $bDontExitOnFailure) = @_;

    Vprint ( $OptVerbose, "System command: \"$command\"");

    if ( $log && defined $gLogFile )
    {
       system ( "$command 2>&1 >> \"$gLogFile\"" );
    }
    else
    {
       system ( "$command" );
    }

    my $status = $? >> 8;
    if ( $status )
    {
        unless( $bDontExitOnFailure )
        {

           Error( "System Command ($command) failed. Status = $status" );
        }
        Vprint( $log, "System Command ($command) failed. Status = $status" );
        return FALSE;
    }
    return TRUE;
}

######################################
# prints the specified message to the log and optionally to the screen
#
######################################
sub Vprint
{
   my ($log, $message) = @_;

   ## print the message to the screen if option says to
   if ( $log )
   {
      print "BUILD: $message\n";
   }

   if ( !defined $gLogFile)
   {
      return;
   }
   ## create the log file if it does not exist
   unless (-e $gLogFile)
   {
      open(LOGFILE, "+> $gLogFile")
         or print("ERROR: Cannot open file: $gLogFile for create: $!");
      print LOGFILE "Automatic Build Log - $gDate $gTime\n";
      close LOGFILE;
   }

   ## always print to the log file
   if ( open(LOGFILE, ">> $gLogFile") )
   {
      print LOGFILE "BUILD: $message\n";
      close LOGFILE;
   }
   else
   {
      #print("ERROR: Cannot open file: $gLogFile for append: $!\n");
      print("PRINT: $message\n");
   }

}

######################################
# Move the log file to the specified location
#
# Param: new location for the log file
######################################
sub PerformLog
{
   my $line = $_[0];
   if ( $line =~ /^\s*log\s*"(.*)"\s*$/i )
   {
      Vprint( LOG, "Moving log file from: '$gLogFile' to : '$1'" );
      $gLogFile = $1;
      unlink $gLogFile;
   }
   else
   {
      Error( "Bad sytax in Log: $line" );
   }
}

######################################
# Searches through the arguments for the Define Keyword
# then replaces the values throughout the lines
# also removes comments and handles the log file
#
# Param: stringArray An array of lines of a configuration file
#
# Return: Returns the lines with no Define statements and with all variables replaced with values
#
######################################
sub PreProcessConfiguration
{
   my @lines = @_;

   my %gVariables;

   Vprint ( $OptVerbose, "Checking for Global Variables" );

   foreach( @lines )
   {
      ## check for Global variables (no whitepace before the word 'Define')
      if ( $_ =~ /^Define\s+/i )
      {
         PerformDefine( $_ );

         ## remove global defines from lines
         $_ = "";
      }

      ### check for (and remove) comments
      if ( $_ =~ /^\s*\/\/.*$/ )
      {
         Vprint( $OptVerbose, "removing comment: $_" );

         $_ = "";
      }
   }

   foreach( @lines )
   {
      $_ = ProcessForVariables( $_ );
   }

   if ( $OptVerbose )
   {
      Vprint ( 1, "ConfigFile after removing comments and substituting global variables:" );
      Vprint ( 1, "---------------------------------------------------------------------" );
      foreach( @lines )
      {
         Vprint( 1, "$_" );
      }
      Vprint ( 1, "---------------------------------------------------------------------" );
   }

   return @lines;
}

######################################
# Adds the specified variable with the specified value, resets variable if it already exists
#
# Param: string The variable name
# Param: string The value for the variable
#
######################################
sub AddVariable
{
   my ($var, $value) = @_;
   $gVariables{ $var } = $value;
}

######################################
# Returns the value of a variable, or undef if the variable is not defined
#
# Param: string The variable name to search for
# Return: The value of a variable, or undef if the variable does not exist or is not defined
######################################
sub GetVariable
{
   my ( $var ) = @_;

   $var = "\U$var";

   if ( not exists $gVariables{ $var } )
   {
      Vprint( $OptVerbose, "Variable $var is not defined" );
      return undef;
   }
   elsif( not defined $gVariables{ $var } )
   {
      Vprint( $OptVerbose, "Variable $var is not defined" );
      return undef;
   }
   else
   {
      Vprint( $OptVerbose, "Variable $var exists and equals: $gVariables{$var}" );
      return $gVariables{ $var };
   }
}

######################################
# Searches the given line for any variables that need to be replaced
#
# Param: string a line of the configuration file
#
# Return: string The given line, but with variables replaced
#
######################################
sub ProcessForVariables
{
   my $line = $_[0];

   ## initially set to true this is so the loop happens the first time
   ## the loop will continue as long as a substitution has been made
   my $bMayHaveVariables = "true";
   while ( $bMayHaveVariables eq "true" )
   {
      ## assume there are no more variables
      $bMayHaveVariables = "false";

      for my $variable ( keys %gVariables )
      {
         if ( $line =~ /.*\[$variable\].*/ )
         {
            ## since we found a variable, and are doing a substitution, we could introduce more variables
            ## so make sure we check again
            $bMayHaveVariables = "true";

            my $value = $gVariables{$variable};

            ## since the line contains this variable, replace all instances of it
            $line =~ s/\[$variable\]/$value/;
         }
      } ## end for
   } ## end unless

   return "$line";
}

######################################
# Adds the P4 data to the hash
#
# Param: string Unique name to identify the data / settings as
# Param: string The root directory for the perforce server
# Param: string The clientspec
# Param: string The server name and port
# Param: string The user to access the server as
#
######################################
sub AddP4Settings
{
    my ($P4KeyName, $P4Root, $P4Client, $P4Server, $P4User) = @_;

    my @keys = keys %gP4Settings;
    foreach( @keys )
    {
       my $key = $_;
       if ( "$key" eq "$P4KeyName" )
       {
          Error( "$P4KeyName already exists as a perforce settings. Please ensure you are using the correct settings." );
       }
    }

    my $P4Setting = P4Setting->new();
    $P4Setting->root( $P4Root );
    $P4Setting->client( $P4Client );
    $P4Setting->server( $P4Server );
    $P4Setting->user( $P4User );

    Vprint( $OptVerbose, "Adding P4Setting $P4KeyName with root $P4Root; client $P4Client; server $P4Server; user $P4User" );
    $gP4Settings{ $P4KeyName } = $P4Setting;
}

######################################
# Removes the P4 data from the hash
#
# Param: string Unique name to identify the data / settings
#
######################################
sub RemoveP4Settings
{
   my $P4KeyName = $_[0];

   my $P4Setting = P4Setting->new();
   $P4Setting = $gP4Settings{ $P4KeyName };
   if (not defined $P4Setting )
   {
      Vprint( LOG, "Cannot remove P4Settings: Invalid Perforce reference ID: $P4KeyName " );
      return;
   }

   my $root = $P4Setting->root;
   my $client = $P4Setting->client;
   my $server = $P4Setting->server;
   my $user = $P4Setting->user;

   Vprint( $OptVerbose, "Removing P4Setting $P4KeyName with root $root; client $client; server $server; user $user" );
   delete($gP4Settings{$P4KeyName});

}

######################################
# returns the settings associated with the P4 id
#
# Param: string Unique name associated with desired perforce data
#
# Return: string The root directory
# Return: string The clientspec
# Return: string The server name and port
# Return: string The user to access the server as
#
######################################
sub GetP4Settings
{
   my $P4KeyName = $_[0];

   my $P4Setting = P4Setting->new();

   $P4Setting = $gP4Settings{ $P4KeyName };

   if (not defined $P4Setting )
   {
      Error ( "Invalid Perforce reference ID: $P4KeyName " );
   }

   my $root = $P4Setting->root;
   my $client = $P4Setting->client;
   my $server = $P4Setting->server;
   my $user = $P4Setting->user;

   if ( not defined $root or
        not defined $client or
        not defined $server or
        not defined $user )
   {
      Error( "Invalid Perforce ID: $P4KeyName" );
   }

   return ( $root, $client, $server, $user );
}

######################################
# prints usage of the script and exits
#
######################################
sub PrintHelpAndExit
{
    print $UsageText;
    exit;
}

######################################
# Adds preprocessor defines to the specified project of the specified solution
#
# Param: Path to the solution file
# Param: The name of the project to modify; "" will update all projects
# Param: The configuration to add the defines to
# Param: The Define macros that should be added
#
######################################
my @gBackupVcprojFiles;
sub AddDefinesToSln
{
   my $solution = $_[0];
   my $project = $_[1];
   my $configuration = $_[2];
   my $defines = $_[3];

   Vprint( LOG, "Adding defines '$defines' to vcproj files in $solution" );

   ## 1. open the solution and find the project files
   open( SLNFILE, "$solution" ) || Error( "Cannot open Solution for reading: $solution" );
   chomp( my @slnLines = <SLNFILE> );
   close SLNFILE;

   ## 2. verify the solution is VS 2005 (9.0)
   my $strVSVersion = "";
   foreach( @slnLines)
   {
      if ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s8\.00/ )
      {
         ## Visual Studio 2003
         $strVSVersion = "2003";
      }
      elsif ( $_ =~ /Microsoft\sVisual\sStudio\sSolution\sFile,\sFormat\sVersion\s9\.00/ )
      {
         ## Visual Studio 2005
         $strVSVersion = "2005";
      }
   }

   if ( $strVSVersion ne "2005" )
   {
      Error( "Cannot add defines to Non-Visual Studio 2005 projects" );
   }

   ## 3. find the project file(s)
   ## format is like this:
   ## Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "GPUPerfStudio", "MainApp\PerfDash.vcproj", "{A68DAE26-DDE0-4509-9A45-55EEAC341D5A}"
   my @projFiles;
   foreach( @slnLines)
   {
      if ( $project eq "" )
      {
         ## no project is defined, so update all of them
         if ( $_ =~ /^Project\(\"\{.{36}\}\"\)\s=\s\"[^\"]*\",\s\"([^\"]*\.vcproj)\"\,\s\"\{.{36}\}\"$/ )
         {
            push( @projFiles, $1 );
         }
      }
      else
      {
         ## only update the specific project
         if ( $_ =~ /^Project\(\"\{.{36}\}\"\)\s=\s\"$project\",\s\"([^\"]*\.vcproj)\"\,\s\"\{.{36}\}\"$/ )
         {
            push( @projFiles, $1 );
         }
      }
   }


   ## clean up the project file paths to include the path to the solution
   if ( $solution =~ /^(.*[\\\/])[^\\\/]+\.sln$/ )
   {
      ## catch everything except the filename of the solution
      my $slnPath = $1;

      foreach ( @projFiles )
      {
         $_ = "$slnPath$_";
      }
   }

   ## copy the project files so that they can be restored after the build
   foreach ( @projFiles )
   {
      copy( $_, $_."bldbak" ); ##bldbak = build backup
      push( @gBackupVcprojFiles, $_."bldbak" );
      chmod 0777, $_;
   }

   ## 4. Open the project file(s)
   foreach( @projFiles )
   {
      Vprint( $OptVerbose, "Attempting to add to: $_" );
      my $file = $_;

      unless ( -e $file )
      {
         Error( "Cannot add defines to $file: file does not exist" );
      }

      open( VCPROJ, "$file" );
      chomp( my @lines = <VCPROJ> );
      close( VCPROJ );

      my @newLines;

      ## 5. find the <configuration tag
      ## 6. make sure the configuration is for this build configuration
      ## 7. find the compile options tag
      ## 8. insert the new defines

      ##<Configurations>
      ##   <Configuration
      ##      Name="Release|Win32"
      ##      ... (a few lines down)
      ##         <Tool
      ##            Name="VCCLCompilerTool"
      my $bInConfigs                  = "false";
      my $bCheckNextLineForConfigName = "false";
      my $bCheckForTools              = "false";
      my $bCheckNextLineForCompiler   = "false";
      my $bCheckForPreprocessorDefines= "false";

      my @insertLines;

      foreach ( @lines )
      {
         my $origLine = $_;
         ##<Configurations>
         if ( $origLine =~ /^\s*\<Configurations\>\s*$/ )
         {
#            Vprint( $OptVerbose, "In Configurations" );
            $bInConfigs = "true";
         }

         if ( $origLine =~ /^\s*\<\/Configurations\>\s*$/ )
         {
#            Vprint( $OptVerbose, "Leaving Configurations" );
            $bInConfigs = "false";
         }

         if ( $bInConfigs eq "true" )
         {
            ##   <Configuration
            ##      Name="Release|Win32"
            if ( $bCheckNextLineForConfigName eq "true" )
            {
#               Vprint( $OptVerbose, $origLine );

               $bCheckNextLineForConfigName = "false";

               if ( $origLine =~ /^\s*Name=\"$configuration\"\s*$/ )
               {
                  ## in the XML under the proper configuration
                  ## Set the flag to look for the Compiler tool options so that we can add the defines

                  $bCheckForTools = "true";
#                  Vprint( $OptVerbose, "Matched '$configuration' - Checking For Tools" );
               }
               elsif ( $origLine =~ /^\s*Name=\"$configuration\|[^\"]*\"\s*$/ )
               {
                  ## in the XML under the proper configuration
                  ## Set the flag to look for the Compiler tool options so that we can add the defines

                  $bCheckForTools = "true";
#                  Vprint( $OptVerbose, "Matched '$configuration|...'" );
               }
            }

            ##      <Tool
            ##         Name="VCCLCompilerTool"
            if ( $bCheckNextLineForCompiler eq "true" )
            {
#               Vprint( $OptVerbose, "      Checking NextLine For VCCLCompilerTool" );

               $bCheckNextLineForCompiler = "false";

               if ( $origLine =~ /^\s*Name=\"VCCLCompilerTool\"\s*$/ )
               {
#                  Vprint( $OptVerbose, "      - Found VCCLCompilerTool" );
                   ## check to see if preprocessorDefinitions are already defined
                   $bCheckForPreprocessorDefines = "true";
               }
               else
               {
#                  Vprint( $OptVerbose, "      - NOT FOUND VCCLCompilerTool: $origLine" );
               }
            }

            ##         PreprocessorDefinitions="WIN32;_WINDOWS;_DEBUG;ATI_EXPORT;XML_DTD;XML_STATIC"
            ##         ...
            ##      /> (this is the closing tag of of <Tool
            if ( $bCheckForPreprocessorDefines eq "true" )
            {
               ## if this line is the closing tag
               if ( $origLine =~ /^\s*\/\>\s*$/ )
               {
                  ## we didn't find the preprocessorDefinitions line
                  ## so insert it
                  push( @insertLines, "\t\t\t\tPreprocessorDefinitions=\"$defines\"" );

                  ## and don't check for the defines again
                  $bCheckForPreprocessorDefines = "false";
                  $bCheckForTools = "false";
               }
               else
               {
                  ## check if this line already contains the preprocessor definitions
                  if ( $origLine =~ /^(\s*PreprocessorDefinitions=\")([^\"]*\"\s*)$/ )
                  {
                     ## this line has preprocessorDefinitions
                     ## so insert the new ones
                     $origLine = "$1$defines\;$2";

                     ## and don't check for the defines again
                     $bCheckForPreprocessorDefines = "false";
                     $bCheckForTools = "false";
                  }
                  else
                  {
                     ## not a preprocessor definition
                  }
               }
            }

            ##      <Tool
            ##         Name="VCCLCompilerTool"
            if ( $bCheckForTools eq "true" )
            {
#               Vprint( $OptVerbose, "   Checking For Tools" );
               if( $origLine =~ /^\s*\<Tool\s*$/ )
               {
                  $bCheckNextLineForCompiler = "true";
#                  Vprint( $OptVerbose, "   - Matched!" );
               }
            }

            ##   <Configuration
            ##      Name="Release|Win32"
            if ( $origLine =~ /^\s*\<Configuration\s*$/ )
            {
               $bCheckNextLineForConfigName = "true";
#               Vprint( $OptVerbose, "Checking NextLine for ConfigName '$configuration'" );
            }
         }

         ## insert any new lines first
         foreach ( @insertLines )
         {
            my $insertLine = $_;
            Vprint( $OptVerbose, "ADDING LINE: $insertLine" );
            push( @newLines, $insertLine );
         }
         @insertLines = ();

         ## add original line to the newlines
         push( @newLines, $origLine );
      }

      ## 9. write the project file back out
      unlink $file;
      open( VCPROJOUT, ">$file" ) || Error( "Cannot open $file for writing");
      foreach ( @newLines )
      {
         print VCPROJOUT "$_\n";
      }
      close VCPROJOUT;
   }
}

## restores the vcproj files after the defines were added
sub UndoAddDefinesToSln
{
   foreach( @gBackupVcprojFiles )
   {
      my $origName = $_;
      $origName =~ s/\.vcprojbldbak/\.vcproj/;

      Vprint( LOG, "Restoring preprocessor definitions for: $origName" );

      unlink $origName;
      copy( $_, $origName ) or Error( "Could not restore $origName from $_" );
      unlink( $_ );
   }

   ## clear the list of files to restore
   @gBackupVcprojFiles = ();
}


#############################################################
# Prepends a text file to one (or a set of) text files.  This
# is used, for example, to prepend a license to the top of
# a set of *.cpp/*.h files
#
# Param: string Path to the files to update
# Param: string File(s) to update (can be wildcard)
# Param: string Path to text file to prepend
#
##############################################################
sub PerformPrependText
{
   my $line = $_[0];

   my $filePath;
   my $fileName;
   my $prependTextFile;

   if ( $line =~ /^[^\"]*"([^\"]*)"\s*"([^\"]*)"\s*"([^\"]*)"\s*$/ )
   {
       $filePath = $1;
       $fileName = $2;
       $prependTextFile = $3;
   }
   else
   {
      Error( "Bad syntax in PrependText: '$line'" );
   }

   if ( $filePath =~ /^\s*$/ )
   {
      Error( "Path to file not specified in PrependText" );
   }

   if ( $fileName =~ /^\s*$/ )
   {
      Error ( "File not specified in PrependText" );
   }

   if ( $prependTextFile =~ /^\s*$/ )
   {
      Error ( "Text file to prepend not specified in PrependText" );
   }

   # Set the global file filter and file name
   $gPrependTextFile = $prependTextFile;
   $gPrependTextFileFilter = $fileName;

   # Call File::Find, which will be recursed on all files/dirs in
   # the path
   find( \&PrependTextToFile, "$filePath" );
}

##############################################################
# This callback is used by File::Find.  On each file, it
# prepends the $gPrependTextFile to the files that match
# $gPrependTextFileFilter.  The global variables are setup
# by the PerformPrependText subroutine.  This function is
# recursed on all files.
##############################################################
sub PrependTextToFile
{
   # If this is a directory, then examine the files.
   if( -d $_ )
   {
      # Find all files that match pattern within this directory
      my @matchingFiles;
      push ( @matchingFiles, bsd_glob("$_\\$gPrependTextFileFilter") );

      # Open the prepend text file
      open( FILEIN, $gPrependTextFile ) or
            Error( "Cannot open file: $gPrependTextFile for reading: $!" );

      chomp (my @prependLines = <FILEIN>);
      close FILEIN;

      # On each match file, prepend the text
      foreach ( @matchingFiles )
      {
         my $match = $_;

         Vprint( LOG, "Prepending text to $match" );

         # Make sure that the file is writeable, if not, make it so
         if ( -w "$match" )
         {
            ## file is already writeable
         }
         else
         {
            unless( chmod( 0666, $match ) )
            {
               Vprint( LOG, "Could not chmod '$match': $!" );
            }
         }

         open( FILEIN, $match ) or
             Error( "Cannot open file: $match for reading: $!" );

         chomp (my @lines = <FILEIN>);
         close FILEIN;

         open (SAMEFILE,">", $match) or
             Error( "PerformPrependText::Cannot open file to: $match $!" );

         # Add prepended text
         foreach( @prependLines )
         {
            print SAMEFILE "$_\n";
         }

         # Add original file text
         foreach( @lines )
         {
            print SAMEFILE "$_\n";
         }

         close ( SAMEFILE );
      }
   }
}

#############################################################
# Waits for the specified file to exist on disk (by sleeping
# for a second and checking to see if the file exists).
# Times out after 3 minutes (360 iterations)
#
# Param: string Path to the file to wait for
#
##############################################################
sub PerformWaitForFileExist
{
   my $line = $_[0];
     
   if ( $line =~ /^[^\"]*"(.*)"\s*$/ )
   {
      my $file = $1;
      my $count = 1;
      while ( !(-e $file ) )
      {
         $count++;      
         if ( $count > 360 )
         {
            Error( "Timed out waiting for file: $file" );
            return;
         }
         sleep(1);
      }
   }
   else
   {
      Error( "Bad syntax in PerformWaitForFileExist: '$line'" );
   }
}
#############################################################
# Report the system time to the log, uses 
# Syntax in a config file:
#  ReportSystemTime
#############################################################
sub PerformReportSystemTime
{
   # fetch & split time
   my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();

   my $systime = sprintf "%02s:%02s.%02s", $hour, $min, $sec;
   Vprint( LOG, "System time: $systime" );
}

#############################################################
# Set the timeout fed to NetConsole - this is a global item
# Syntax in a config file:
#  SetTimeout  "<seconds>"
# Param: timeout in seconds
#############################################################
sub PerformSetTimeout
{
   my $line = $_[0];
   if ( $line =~ /^[^\"]*"(.*)"\s*$/ )
   {
      $gTestcaseTimeout = $1;
      Vprint( LOG, "Setting the timeout to be $1" ) ;
   }
   else
   {
      Error( "Bad syntax in SetTimeout: '$line'" );
   }
}
