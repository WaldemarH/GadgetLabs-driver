This driver is provided "as is", without any guarantee made
as to its suitability or fitness for any particular use. It may
contain bugs, so use it at your own risk. I take no responsibility
for any damage that may unintentionally be caused through its use.

**************************************************************************************************************************
**************************************************************************************************************************
Name   : GadgetLabs WavePro series windows driver

Systems: XP SP2 and later

Version: 4.2.0.2:  - 32/64 bit OS support
                   - up to 4 cards in system
                   - FullDuplex channel support
                   - 24bit playback/recording support
                   - sample rates: 44100, 48000, ( 88200, 96000 ) (system supports others with resampling)
                   - daughter cards will never be supported
                   - different buffer sizes
                   - stereo/all channels
                     Notice:
                     All channel is used for surround support. If All channel is opened it means that all
                     the inputs and/or outputs are reserved for it. So you can have up to 7+1 surround
                     support on 824 cards.
                   - Full Graphic User Interface
                   - MIDI driver

date: 16.01.2011

by: Waldemar Haszlakiewicz

For any help/support join the GadgetLabs group: http://tech.groups.yahoo.com/group/gadgetlabs2/

**************************************************************************************************************************
**************************************************************************************************************************
Thank you-s:

1st things first we all should thank Jay because if he wouldn't support us I doubt there would be any driver for Windows.

2nd We have to remember another good Gadgeter and our friend James Manning.
    You were one of the founding fathers of wonderful Gadgetlabs comunity and we will always be in debt for love that you shared.
    We love you man and we'll see you in heaven... first round is on me.

3rd For the old days I have to thank the whole GL Love Teem which send me a 824 card. :D
    A special thanks goes to 'white' previously known as 'gray' -> it responds also to 'massive brain'. ;)
    And to Patrice Payen for his mighty mighty tasting chocolates. :)

Love you all. :)


The whole GL Love Teem (alphabetically -> except for Sky ;) ):
Sky "The GL girl"

Allen Myers
Andy McDonough
Bob Watson / Blind Reality
Chalmers Davis
Chris Short "Red"
Cliff .S. Russell
Come Together Music
CSR / S-O Taryk
D'Arcy Rittich
Dean Goble
Dick Winograde
Doug Proctor / Skunkworks Audio
Eric Beam
Eric Harrington
Ex-gadgeteer Jay S. / emmaker
Francois Groulx
Franksterman FR
Gene Maruszewski
Geoff Collier
gray systems deep / gray / massive brain Ink.
Hans van Andel
Hartmut Paul
James Badger jb
James Edward Manning
Jay A. A.K.A. Control Dude
Jeff D.
Jeff Eyberse
Jethro
Jim / Melanie Wacker
Jim Stewart
Joergen Roskjaer
Julio Aguirre
Kiko justsayspotcom
Kyle Dunn
Larry McDonald MCINCUBUS
Leny
Maak Bow
Mantis
Marty Nelson
Mr. Bloke
Nic Ventura
obiwan
Paul / Mary Freudental
Paul Hartmut
Pauly
Patrice Payen
Phillip V. Heijningen
pushkar
PsySpyder
Richard Marshall  A.K.A. Critical Mass
Robert Ludwick, Hinterwaldler Studio
Robert Watson
Robin Ives backlighting-UK
Russell Kalman
ScotcH
Shiu Kwok Kuen
Tony Hoare
Udita J
Uihs yennek
Vafam Sound
Vincent Colletti / Custom Stringed Instruments
Wendy Hutton
White Light Tech. Inc
Wim Prest

**************************************************************************************************************************
**************************************************************************************************************************
Instructions:

1st Installing/Uninstalling:
    - How to install: check the 'how to install.txt'.

    - Multicard system : - YOU HAVE TO install/uninstall all the cards at the same time if you wish
                           that card serial number are correct.
                           Or you can install one now and one after whatever time, but you will have to
                           reset the machine.
                         - You can install only 1 card if you want and disable others.
     
2nd Don't drink and drive!!!

3rd Love all world people, we are all part of this small vilage called Earth.

**************************************************************************************************************************
**************************************************************************************************************************
Errors that can happen:

1st Aaammmmmm ???? mmmmmmm ????? hmmmmmmm  Well that's it. :)

**************************************************************************************************************************
**************************************************************************************************************************
News:
4.2.0.2   - Fixed the 824 not working bug -> code was specially designed for 4xx cards MIDI testing -> removed test code.
          - Disabled automatic sync in GUI -> need to come with some good way to do it... the problem is Win7 which
            opens channels in the background.

4.2.0.1   - Unsync data is reset on card uninstalled.

4.2.0.0   - Fixed some minor issues in GUI.
          - Added specific card data to driver and GUI.
          - Fixed inf file Wow64 32bit ASIO dll problem.

4.1.0.0   - MIDI driver added... needs at least XP SP2.
          - Run->Pause->Run state solved... the reported clock was not correct, so playback didn't start.
          - GUI bug pointed out by Tom324. Thanks man. :)
          - Kernel bug pointed out by Tom324. Thanks man. :)

4.0.0.5   - First public release of the new driver.

4.0.0.x   - Internal versions of the new completely rewritten 32/64 bit driver.

3.4.4.13  - First version of ControlPanel released.

3.4.4.12  - Fixed a bug in Altera clock detector.
          - Fixed a bug in driver sync detecting code.

3.4.4.11  - Added even more of the log stuff.

3.4.4.10  - Added extensive logging on sync code -> needed for detecting the sync problem.

3.4.4.9   - Redesign of the sync code in hardware and software.
          - Driver remembers monitor status when in ASIO driver (it sets it back when ASIO driver closes).

3.4.4.8   - In previous version I've broken part of the MIDI driver, so now I've set it back to the correct state.
          - Fixed communication types in PIN definitions.
          - Midi driver update -> now it supports MIDI direct streaming(I hope :) ).

3.4.4.7   - Fixed Midi open channel procedure.
          - Fixed Midi pin names.
          - AudioIntersectHandler add to audio in channels.
          - Fixed a bug in CancelPendingIoctl_ASIO procedure.
 
3.4.4.6   - Drastically improved Altera code timings.

3.4.4.5   - Improved Altera code timings.

3.4.4.4   - I've changed so many thing I forgot counting.

3.4.4.3   - ? ( Forgot what I did. :) )
3.4.4.2   - ?

3.4.4.1   - New synhronization code -> this time properly done.
          - Fixed a bug in cards sync detection.
          - Code clean up -> a drastic one.
          - Fixed a bug in SetDataFormat (if ASIO channel was opened).
          - DirectX MIDI in works correctly now -> small bug and lots of trouble.

3.4.4.0   - New Altera code added -> lots of stuff was removed from the driver and lots of procedures
            were rewritten -> just toooo many changes to tell here.
         
2.4.4.13  - Fixed a small bug in play a sinus sound proc (needed for ControlPanel).
          - Added a mute function for 4xx cards -> it doesn't mute the whole channel, but just the CODECs.

2.4.4.12  - Added variable to the registry that will allow users to set the waiting time in Altera programming code ->
            so that user could find the delay that it works for him.

2.4.4.11  - Complete ASIO driver redesign (95%) and 15% of lower level driver.
            Changes: - lower driver is ASIO multiclient capable -> ASIO driver isn't yet
                     - lowel driver is now protected against ASIO driver/host crashing as it should be 
                     - 16bit sample support was added

2.4.4.10  - Fixed an ASIO synhro bug.
          - Internal.

2.4.4.9   - Fixed a bug in ASIO monitoring code.
          - Changed the IOCTL_SET_SAMPLE_RATE slightly to allow ControlPanel to change the sample rate.
          - Small change to inf file -> inf file is copied to the inf dir again.

2.4.4.8   - Parts of SetSampleRate rewritten again.
            Fixed BUG in SetClockSource(90% of the procedure rewritten).

2.4.4.7   - Changed some delays for 4xx cards and for 824 ADC part.
          - Rewritten 60% of SetSampleRate procedure -> 824 cards have a little more delay now, but
            the converters should work at their best performance now.
          - Added another IOCTL for ControlPanel sync card definition wizard.

2.4.4.6   - Fixed a small bug in ASIO driver.

2.4.4.5   - Missed to program 1 PLL register -> sampleRate of 88200Hz didn't work.

2.4.4.4   - Rewritten PLL setting procedures -> removed all line pullings and stuff
            (now there are just big delays -> 1ms) -> also rearanged/changed how register get set,
            so that clocks wont be enabled until PLLs are all set.
            All this, in short, means that programming of "PLL" takes 30seconds.
          - Fixed some ASIO driver bugs.

2.4.4.3   - Fixed some ASIO driver bugs.

2.4.4.2   - ASIO driver added (tested on Cubase SX2).
          - Change Altera programming (it takes 9 second to program Altera).
          - Lots of other small things.

1.4.5.21  - Fixed a minor problem in Altera programming code.
          - Fixed a bug in driver uninstall (synced cards).
          - Fixed a small bug copying 24bit packed samples to the card.
          - Changed copyBuffers procedures a little for faster copying.
          - Commented out some MIDI initializing parts -> not needed right now.

1.4.5.20  - Removed/Added some parts of ControlPanel.
          - Internal -> not released.
1.4.5.19  - Bug fix: slight change to SetSampleRate procedure.

1.4.5.18  - Established ControlPanel communication.
          - Cards can synhronize -> option not available to the users -> there are still minor offsets.
          - 4xx cards PLL programming timings changed.

1.4.5.17  - Full optimization tests.
          - Internal -> not released.
1.4.5.16  - Ups, enabled some disabled code back.
          - Internal -> not released.
1.4.5.15  - Somi minor internal changes.
          - Internal -> not released.
1.4.5.14  - Logger changes.
          - Internal -> not released.
1.4.5.13  - Logger changes.
          - Internal -> not released.
1.4.5.12  - Logger changes.
          - Internal -> not released.
1.4.5.11  - Complete redesign of memory synhronization requests.
          - Internal -> not released.
1.4.5.10  - Logger changes.
          - Internal -> not released.

1.4.5.9   - Redesigned sinhronization procedures and variables -> caused lots of changes to whole driver -> removed
            some unsafe code.
          - Fixed a bug in release card procedure.
          - Redesigned stop card process.
          - Fixed start click.

1.4.5.8   - Fixed MemoryTest bug causing bootUp freezes and system instabilities.
          - Internal -> not released.

1.4.5.7   - Logger changes.
          - Internal -> not released.

1.4.5.6   - Logger changes.
          - Internal -> not released.

1.4.5.5   - Logger changes.
          - Internal -> not released.

1.4.5.4   - Reconfigured Altera programming, 4xx codecs code and 4xx PLL code.
          - Starting of 2 master cards in sync is not permitted anymore -> removed some possibly hazardous code.
            Now only the last channel master card will be started.
          - Bug: If a 32bit mono channel was opened with WAVEFORMATEX, driver misinterpreted it as stereo.
          - Some changes to Create, Close and SetDataFormat procedures.
            Now every possible MONO channel type is supported.
          - Bug: If multiple channels were running and that one changed state from RUN->PAUSE->RUN without going
            to "release resources" state driver started all the channels again -> DPC audio engine failed
            because there were more channels to process than there are in system.
          - Removed Sonar reserved sample type -> now the driver uses left justified sample for every application.

1.4.5.3   - Logger changes.
          - Internal -> not released.

1.4.5.2   - Changed 4xx codec code a little again -> changed the codec programming mode -> there were still some strange
            noises coming out of DAC 1.

1.4.5.1   - Altera programming code changed again ->now Altera is programmed every time (but PLL is only 1 time).
          - changed driver interface names
          - fixed the freeze after reset bug
          - changed drivers returning error code -> now system wont freeze any more if error code is returned
          - 4xx codecs code changed a little -> there were some strange noises coming out of DAC 1
          - uninstall procedure has changed -> now you can uninstall cards in any order -> but no channels should be in use.
          - added some deviceIoControl procedures -> you can set gain, monitoring,... 

1.4.4.5.8 - Removed audio safety net -> causing some AUDIO_POSITION problems.
          - Fixed a bug that was already fix some time back, but reappeared because of different compile/link setup.
          - Complete redesign of AUDIO_POSITION.
            It was causing some problems with MediaPlayer and maybe even created some pops because of wrong position.
          - Code clean up.

1.4.4.5.7 - AUDIO_POSITION added.
          - Some changes to audio engine.

1.4.4.5.6 - Different compile/link setup.

1.4.4.5.5 - Rewritten chip programming code, also added some really big delays.
          - Mono support.

1.4.4.5   - Just disabled initialization of daughter cards.

1.4.4.4   - Fixed a real spy bug->it was hidding so good for so long time ....argggg but I found it. :)
            (As the wave engine tries to sync the buffers back to the others if something goes wrong,
            there are some checks and if really everything failed (which would be very, very rare)
            the bug would do it's job->system crash.)
           -BufferSize is back to normal -> so 3ms buffer means 3ms buffer. :)

1.4.4.3   - Fixed a nasty bug in syncStart function.
          - Added support for 16 bit samples.

1.4.4.2   - Enabled 24bit recording.
            Other:
            Very little to the outside, lots from within.

1.4.2.9   - First public beta release.