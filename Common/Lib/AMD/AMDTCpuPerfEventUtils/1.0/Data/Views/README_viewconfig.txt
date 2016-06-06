
Data collection and view configurations at a glance
P.J. Drongowski 
11 June 2007

View configuration design considerations

  * Offer only appropriate views for the event data collected
  * Offer a small number of views (one to five views) at a time
  * Keep the number of columns small
  * Avoid offering two or more views with similar information

Views are separated into two broad categories: assessment and
investigations.

  * Assessment
      + Suitable for overall performance assessment
      + Are generated using the assess.xml general purpose data
        collection configuration
      + May be offered to complement a focus, purpose-specific view
  * Investigation (focus)
      + Are purpose-specific
      + Provide more detail beyond assessment
      + Are generated using one of the "investigate XYZ" data collection
        configurations

Right now, only assess.xml is marked as a "default view."

With the introduction of new unit masks and IBS in GH, some configurations
are processor specific. Three subdirectories (K7, K8 and GH) were created
to hold processor-specific view configuration files.

Changes in GH include:

    0x045  L1 DTLB miss and L2 DTLB hit   New unit masks
    0x046  L1 DTLB and L2 DTLB miss       New unit masks
    0x085  L1 ITLB miss, L2 ITLB miss     New unit masks

New unit mask values were added to the events 0x7d and 0x7e (L2 cache
requests and misses), but these values do not affect the conditions
measured and report through the predefined configurations.


**************************************************
Views offered for each data collection config
**************************************************

assess.xml: Broad assessment of program behavior

  triage_assess.xml     Overall picture of performance
  ipc_assess.xml        Instructions per cycle
  br_assess.xml         Branch (mis)prediction
  dc_assess.xml         Data cache (DC) basic measurements
  dtlb_assess.xml       Level 2 DTLB basic measurements
  misalign_assess.xml   Misaligned memory accesses

branch.xml: Branch behavior focus

  br_assess.xml         Branch (mis)prediction
  taken_focus.xml       Taken branch measurements
  return_focus.xml      Near return (mis)prediction

data_access.xml: Data access focus

  dc_assess.xml         Data cache (DC) basic measurements
  dc_focus.xml          Data cache details
  dtlb_assess.xml       Level 2 DTLB basic measurements
  dtlb_focus.xml        Level 1 and level 2 Data TLB (DTLB) behavior
  misalign_assess.xml   Misaligned memory accesses

inst_access.xml: Instruction access focus

  ic_focus.xml          Instruction cache (IC) behavior
  itlb_focus.xml        Instruction TLB (ITLB) behavior

l2_access.xml: L2 cache focus

  l2_focus.xml          Unified L2 cache behavior


**************************************************
Conventions
**************************************************

View columns showing event samples must be labelled with the event
abbreviation defined in the k8-events.xml file. Abbreviations are consistent
across the public, internal and NDA BKDG. The "All data" view uses these
abbreviations, so all views should refer to these events using the same
abbreviations. Here is a short list of abbreviations for quick reference.

  Event  Abbreviation         Event name
  -----  -------------------  ---------------------------------------------
   0x40  DC accesses          Data cache accesses
   0x41  DC misses            Data cache misses
   0x42  DC refills L2/sys    Data cache refills from L2 or system
   0x43  DC refills sys       Data cache refills from system
   0x45  DTLB L1M L2H         L1 DTLB miss and L2 DTLB hit
   0x46  DTLB L1M L2M         L1 DTLB miss and L2 DTLB miss
   0x47  Misalign access      Misaligned accesses
   0x7D  L2 requests          Requests to L2 cache
   0x7E  L2 misses            L2 cache misses
   0x7F  L2 fill/write        L2 fill/writeback
   0x80  IC fetches           Instruction cache fetches
   0x81  IC misses            Instruction cache misses
   0x84  ITLB L1M L2H         L1 ITLB miss and L2 ITLB hit
   0x85  ITLB L1M L2M         L1 ITLB miss and L2 ITLB miss
   0x76  CPU clocks           CPU clocks not halted (cycles)
   0xC0  Ret inst             Retired instructions
   0xC2  Ret branch           Retired branch instructions
   0xC3  Ret misp branch      Retired mispredicted branch instructions
   0xC4  Ret taken branch     Retired taken branch
   0xC8  Ret near RET         Retired near returns
   0xC9  Ret near RET misp    Retired near returns mispredicted
   0xCA  Ret ind branch misp  Retired indirect branches mispredicted

The words "rate" and "ratio" are reserved for computed measurements and have
the following usage:

  A "rate" expresses the frequency of occurence of an event. It is computed
  as a ratio of an event and retired instructions (the common case) or CPU
  clocks (cycles.)

  A "ratio" is a computed ratio of an event and a related reference event.
  For example, the "DC miss ratio" is the ratio of DC misses over DC accesses.

The assess.xml data collection configuration is called "Assess performance."
Any view associated with this configuration should have "assessment" in
the view name. Assessment views may be offered and shown using data
collected using one of the "Investigate XYZ" data collection configurations.
The assessment views have the following names:

  XML file name        View configuration name
  -------------------  --------------------------------------
  br_assess.xml        Branch assessment
  dc_assess.xml        Data access assessment
  dtlb_assess.xml      DTLB assessment
  ipc_assess.xml       IPC assessment
  misalign_assess.xml  Misalign access assessment
  triage_assess.xml    Overall assessment

The "Investigate XYZ" data collection configurations allow an engineer to
focus on one particular are of program performance. The "Investigate XYZ"
data collection configurations have the following names:

  XML file name        DC configuration name
  ---------------      --------------------------------------
  branch.xml           Investigate branching
  data_access.xml      Investigate data access
  inst_access.xml      Investigate instruction access
  l2_access.xml        Investigate L2 cache access

Investigations produce reports. The views produced from the "Investigate XYZ"
configurations must have the word "report" in the view name. Here is a list
of the view names that are reports:

  XML file name     View configuration name
  ----------------  --------------------------------------
  dc_focus.xml      Data access report
  dtlb_focus.xml    DTLB report
  ic_focus.xml      Instruction access report
  itlb_focus.xml    ITLB report
  l2_focus.xml      L2 access report
  return_focus.xml  Near return report
  taken_focus.xml   Taken branch report

The misaligned access assessment and branch assessment are offered for
basic misaligned access and branch misprediction measurements. Separate
reports for these measurements are not defined and offered as the views
would be redundant.

**************************************************
Data collection configurations at a glance
**************************************************

assess.xml        branch.xml        data_access.xml   inst_access.xml
----------------  ----------------  ----------------  ----------------  
Instructions      Instructions      Instructions      Instructions
CPU_clocks        Branches          DC_accesses       IC_fetches
Branches          Mispred_branches  DC_missess        IC_misses
Mispred_branches  Taken_branches    DC_refills

DC_accesses       Instructions      Instructions      Instructions
DC_misses         Near_returns      DTLB_L1M_L2H      ITLB_L1M_L2H
DTLB_L1M_L2M      Mispred_near_ret  DTLB_L1M_L2M      ITLB_L1M_L2M
Misalign_access   Mispred_indirect  Misalign_access

l2_access.xml
----------------
Instructions
L2_requests
L2_misses
L2_fill_write

***************************************************
View configuration derived measurements
***************************************************

DC configuration: assess.xml

  IPC = Instructions / CPU_clocks                         [assess]
  CPI = CPU_clocks / Instructions                         ipc.xml

  Branch rate = Branches / Instructions                   [assess, branch]
  Branch mispred rate = Mispred_branches / Instructions   branch.xml
  Branch mispred ratio = Mispred_branches / Branches
  Instr per branch = Instructions / Branches

  DC request rate = DC_accesses / Instructions            [assess]
  DC miss rate = DC_misses / Instructions                 dc_assess.xml
  DC miss ratio = DC_misses / DC_accesses

  DTLB requests = DC_accesses                             [assess]
  DTLB L1 request rate = DC_accesses / Instructions       dtlb_assess.xml
  DTLB L1M L2M rate = DTLB_L1M_L2M / Instructions

  Misalign access rate = Misalign_access / Instructions   [assess, data_access]
  Misalign access ratio = Misalign_access / DC_accesses   misalign.xml

DC configuration: branch.xml

  Branch rate = Branches / Instructions                   [assess, branch]
  Branch mispred rate = Mispred_branches / Instructions   branch.xml
  Branch mispred ratio = Mispred_branches / Branches
  Instr per branch = Instructions / Branches

  Taken branch rate = Taken_branches / Instructions       [branch]
  Taken branch ratio = Taken_branches / Branches          taken.xml

  RET mispred rate = Mispred_near_ret / Instructions      [branch]
  RET mispred ratio = Mispred_near_ret / Near_returns     return.xml
  Indirect mispred rate = Mispred_indirect / Instructions
  Instr per call = Instructions / Near_returns

DC configuration: data_access.xml

  DC request rate = DC_accesses / Instructions            [data_access]
  DC miss rate = DC_misses / Instructions                 dc_focus.xml
  DC miss ratio = DC_misses / DC_accesses
  DC refill rate = DC_refills / Instructions
  DC refill ratio = DC_refills / DC_accesses

  DTLB requests = DC_accesses                             [data_access]
  DTLB misses = DTLB_L1M_L2H + DTLB_L1M_L2M               dtlb_focus.xml
  DTLB L1 request rate = DC_accesses / Instructions
  DTLB L1M L2H rate = DTLB_L1M_L2H / Instructions
  DTLB L2M L2M rate = DTLB_L1M_L2M / Instructions

  Misalign access rate = Misalign_access / Instructions   [assess, data_access]
  Misalign access ratio = Misalign_access / DC_accesses   misalign.xml

DC configuration: inst_access.xml

  IC request rate = IC_fetches / Instructions             [inst_access]
  IC miss rate = IC_misses / Instructions                 ic_focus.xml
  IC miss ratio = IC_misses / IC_fetches

  ITLB requests = IC_fetches                              [inst_access]
  ITLB misses = ITLB_L1M_L2H + ITLB_L1M_L2M               itlb_focus.xml
  ITLB L1 request rate = IC_fetches / Instructions
  ITLB L1M L2H rate = ITLB_L1M_L2H / Instructions
  ITLB L2M L2M rate = ITLB_L1M_L2M / Instructions

DC configuration: l2_access.xml

  L2 read request rate = L2_requests / Instructions       [l2_access]
  L2 write request rate = L2_fille_write / Instructions   l2_focus.xml
  L2 miss rate = L2_misses / Instructions
  L2 miss/read ratio = L2_misses / L2_accesses
  L2 miss/write ratio = L2_misses / L2_fill_write

