
proc WriteProjectParameters { basename dir problemtypedir TableDict} {

    ## Source auxiliar procedures
    source [file join $problemtypedir ProjectParametersAuxProcs.tcl]
        
    ## Start ProjectParameters.json file
    set filename [file join $dir ProjectParameters.json]
    set FileVar [open $filename w]
    
    puts $FileVar "\{"

   
    ## AMR data
    puts $FileVar "   \"AMR_data\": \{"
    puts $FileVar "        \"activate_AMR\":                    [GiD_AccessValue get gendata Activate_AMR],"
    puts $FileVar "        \"plane_state\":                    \"[GiD_AccessValue get gendata Plane_state]\","
    puts $FileVar "        \"mesh_optimality_criteria\":       \"[GiD_AccessValue get gendata Mesh_Optimality_Criteria]\","
    puts $FileVar "        \"permissible_error\":               [GiD_AccessValue get gendata Permissible_Error],"
    puts $FileVar "        \"refinement_frequency\":            [GiD_AccessValue get gendata Refinement_Frequency],"
	puts $FileVar "        \"Mapping_Procedure\":              \"[GiD_AccessValue get gendata Mapping_Procedure]\","
    puts $FileVar "        \"gid_path\":                       \"[GiD_AccessValue get gendata gid_path]\""
    puts $FileVar "    \},"
    ## problem_data
    puts $FileVar "   \"problem_data\": \{"
    puts $FileVar "        \"problem_name\":         \"$basename\","
    puts $FileVar "        \"model_part_name\":      \"Structure\","
    puts $FileVar "        \"domain_size\":          [GiD_AccessValue get gendata Domain_Size],"
    puts $FileVar "        \"start_time\":           [GiD_AccessValue get gendata Start_Time],"
    puts $FileVar "        \"end_time\":             [GiD_AccessValue get gendata End_Time],"
    puts $FileVar "        \"time_step\":            [GiD_AccessValue get gendata Delta_Time],"
	puts $FileVar "        \"echo_level\":           [GiD_AccessValue get gendata Echo_Level]"
    puts $FileVar "    \},"
    ## solver_settings
    puts $FileVar "   \"solver_settings\": \{"
    if {[GiD_AccessValue get gendata Solution_Type] eq "Static"} {
        puts $FileVar "            \"solver_type\":                       \"solid_mechanics_static_solver\","
        puts $FileVar "            \"solution_type\":                     \"Static\","
        puts $FileVar "            \"analysis_type\":                     \"Non-Linear\","
    } else {
        puts $FileVar "            \"solver_type\":                       \"solid_mechanics_implicit_dynamic_solver\","
        puts $FileVar "            \"solution_type\":                     \"Dynamic\","
        puts $FileVar "            \"time_integration_method\":           \"Implicit\","
        puts $FileVar "            \"scheme_type\":                       \"Newmark\","
    }
	puts $FileVar "            \"echo_level\":                         [GiD_AccessValue get gendata Echo_Level],"
    puts $FileVar "            \"model_import_settings\":              \{"
    puts $FileVar "                 \"input_type\":         \"mdpa\","
    puts $FileVar "                 \"input_filename\":     \"$basename\","
    puts $FileVar "                 \"input_file_label\":    0"
    puts $FileVar "            \},"
    puts $FileVar "            \"line_search\":                          [GiD_AccessValue get gendata Line_search],"
    puts $FileVar "            \"convergence_criterion\":               \"[GiD_AccessValue get gendata Convergence_Criterion]\","
    puts $FileVar "            \"displacement_relative_tolerance\":      [GiD_AccessValue get gendata Displacement_Relative_Tolerance],"
    puts $FileVar "            \"displacement_absolute_tolerance\":      [GiD_AccessValue get gendata Displacement_Absolute_Tolerance],"
    puts $FileVar "            \"residual_relative_tolerance\":          [GiD_AccessValue get gendata Residual_Relative_Tolerance],"
    puts $FileVar "            \"residual_absolute_tolerance\":          [GiD_AccessValue get gendata Residual_Absolute_Tolerance],"
    puts $FileVar "            \"max_iteration\":                        [GiD_AccessValue get gendata Max_Iterations],"

    puts $FileVar "            \"linear_solver_settings\":     \{"
    puts $FileVar "                 \"solver_type\":      \"[GiD_AccessValue get gendata Solver_Type]\","
    puts $FileVar "                 \"scaling\":           false"
    puts $FileVar "            \},"

	set PutStrings \[
    set BGroups [GiD_Info conditions Body_Part groups]

    # Body_Part
    if {[llength $BGroups] eq "1"} {
    AppendGroupName PutStrings Body_Part
    } else {
    AppendGroupNames PutStrings Body_Part
    }
    
    append PutStrings \]
    puts $FileVar "        \"problem_domain_sub_model_part_list\": $PutStrings,"
   
    ## processes_sub_model_part_list
    set PutStrings \[
    # Solid_Displacement
    AppendGroupNames PutStrings Solid_Displacement
    # Force
    AppendGroupNames PutStrings Force
    # Face_Load
    AppendGroupNames PutStrings Face_Load
    # Normal_Load
    AppendGroupNames PutStrings Normal_Load
    # Body_Acceleration
    AppendGroupNames PutStrings Body_Acceleration

    set PutStrings [string trimright $PutStrings ,]
    append PutStrings \]
    puts $FileVar "        \"processes_sub_model_part_list\":      $PutStrings"
    puts $FileVar "   \}," 
    ## body_domain_sub_model_part_list
    set PutStrings \[
    AppendGroupNames PutStrings Body_Part
    set PutStrings [string trimright $PutStrings ,]
    append PutStrings \]   

    #puts $FileVar "        \"body_domain_sub_model_part_list\":    $PutStrings,"
    ## loads_sub_model_part_list
    #set PutStrings \[
    #set iGroup 0
    # Force
    #AppendGroupNamesWithNum PutStrings iGroup Force
    # Face_Load
    #AppendGroupNamesWithNum PutStrings iGroup Face_Load
    # Normal_Load
   # AppendGroupNamesWithNum PutStrings iGroup Normal_Load
    # Body_Acceleration
    #AppendGroupNamesWithNum PutStrings iGroup Body_Acceleration
    #if {$iGroup > 0} {
    #    set PutStrings [string trimright $PutStrings ,]
    #}
    #append PutStrings \]
    #puts $FileVar "        \"loads_sub_model_part_list\":          $PutStrings,"
    ## loads_variable_list
    #set PutStrings \[
    # Force
    #AppendGroupVariables PutStrings Force POINT_LOAD
    # Face_Load
    #AppendGroupVariables PutStrings Face_Load LINE_LOAD
    # Normal_Load
    #AppendGroupVariables PutStrings Normal_Load POSITIVE_FACE_PRESSURE
    # Body_Acceleration
    #AppendGroupVariables PutStrings Body_Acceleration VOLUME_ACCELERATION
    #if {$iGroup > 0} {
    #    set PutStrings [string trimright $PutStrings ,]
    #}
    #append PutStrings \]
    #puts $FileVar "        \"loads_variable_list\":                $PutStrings"
    #puts $FileVar "    \},"

    

    ## constraints_process_list
    set Groups [GiD_Info conditions Solid_Displacement groups]
    set NumGroups [llength $Groups]
    set iGroup 0
    puts $FileVar "    \"constraints_process_list\": \[\{"
    # Solid_Displacement
    set Groups [GiD_Info conditions Solid_Displacement groups]
    WriteConstraintVectorProcess FileVar iGroup $Groups lines DISPLACEMENT $TableDict $NumGroups
    WriteConstraintVectorProcess FileVar iGroup $Groups points DISPLACEMENT $TableDict $NumGroups

    ## loads_process_list
    set Groups [GiD_Info conditions Force groups]
    set NumGroups [llength $Groups]
    set Groups [GiD_Info conditions Face_Load groups]
    incr NumGroups [llength $Groups]
    set Groups [GiD_Info conditions Normal_Load groups]
    incr NumGroups [llength $Groups]
    set Groups [GiD_Info conditions Body_Acceleration groups]
    incr NumGroups [llength $Groups]

    if {$NumGroups > 0} {
        set iGroup 0
        puts $FileVar "    \"loads_process_list\": \[\{"
        # Force
        set Groups [GiD_Info conditions Force groups]
        WriteLoadVectorProcess FileVar iGroup $Groups POINT_LOAD $TableDict $NumGroups
        # Face_Load
        set Groups [GiD_Info conditions Face_Load groups]
        WriteLoadVectorProcess FileVar iGroup $Groups LINE_LOAD $TableDict $NumGroups

        # Normal_Load
        set Groups [GiD_Info conditions Normal_Load groups]
        WriteNormalLoadProcess FileVar iGroup $Groups POSITIVE_FACE_PRESSURE $TableDict $NumGroups

        # Body_Acceleration
        set Groups [GiD_Info conditions Body_Acceleration groups]
        WriteGLoadVectorProcess FileVar iGroup $Groups VOLUME_ACCELERATION $TableDict $NumGroups
    } else {
        puts $FileVar "    \"loads_process_list\":       \[\],"
    }
    
    ## output_configuration
    puts $FileVar "    \"output_configuration\": \{"
    puts $FileVar "        \"result_file_configuration\": \{"
    puts $FileVar "            \"gidpost_flags\":       \{"
    puts $FileVar "                \"WriteDeformedMeshFlag\": \"[GiD_AccessValue get gendata Write_deformed_mesh]\","
    puts $FileVar "                \"WriteConditionsFlag\":   \"[GiD_AccessValue get gendata Write_conditions]\","
    puts $FileVar "                \"GiDPostMode\":           \"GiD_PostAscii\","
    puts $FileVar "                \"MultiFileFlag\":         \"MultipleFiles\""
    puts $FileVar "            \},"
    puts $FileVar "            \"file_label\":          \"[GiD_AccessValue get gendata File_label]\","
    puts $FileVar "            \"output_control_type\": \"[GiD_AccessValue get gendata Output_control_type]\","
    puts $FileVar "            \"output_frequency\":     [GiD_AccessValue get gendata Output_frequency],"
    puts $FileVar "            \"body_output\":          [GiD_AccessValue get gendata Body_output],"
    puts $FileVar "            \"node_output\":          [GiD_AccessValue get gendata Node_output],"
    puts $FileVar "            \"skin_output\":          [GiD_AccessValue get gendata Skin_output],"
    puts $FileVar "            \"plane_output\":         \[\],"
    
    # nodal_results
    set PutStrings \[
    set iGroup 0
    AppendOutputVariables PutStrings iGroup Write_Solid_Displacement DISPLACEMENT
    AppendOutputVariables PutStrings iGroup Write_Solid_Displacement VELOCITY
    AppendOutputVariables PutStrings iGroup Write_Solid_Displacement ACCELERATION
    if {[GiD_AccessValue get gendata Write_Reactions] eq true} {
        incr iGroup
        append PutStrings \" REACTION \"  ,
    }

    #AppendOutputVariables PutStrings iGroup Write_Force POINT_LOAD
    #AppendOutputVariables PutStrings iGroup Write_Face_Load LINE_LOAD
    #AppendOutputVariables PutStrings iGroup Write_Normal_Load POSITIVE_FACE_PRESSURE
    #AppendOutputVariables PutStrings iGroup Write_Body_Acceleration VOLUME_ACCELERATION
    if {$iGroup > 0} {
        set PutStrings [string trimright $PutStrings ,]
    }
    append PutStrings \]
    puts $FileVar "            \"nodal_results\":       $PutStrings,"

    # gauss_point_results
    set PutStrings \[
    set iGroup 0
    AppendOutputVariables PutStrings iGroup Write_Strain GREEN_LAGRANGE_STRAIN_TENSOR
    AppendOutputVariables PutStrings iGroup Write_Predictive_Stress CAUCHY_STRESS_TENSOR
    AppendOutputVariables PutStrings iGroup Write_Predictive_Stress STRESS_VECTOR
    AppendOutputVariables PutStrings iGroup Write_Integrated_Stress STRESS_VECTOR_INTEGRATED
    AppendOutputVariables PutStrings iGroup Write_Damage DAMAGE_ELEMENT
    AppendOutputVariables PutStrings iGroup Write_Is_Damaged IS_DAMAGED
    AppendOutputVariables PutStrings iGroup Stress_Threshold STRESS_THRESHOLD

    if {$iGroup > 0} {
        set PutStrings [string trimright $PutStrings ,]
    }
    append PutStrings \]
    puts $FileVar "            \"gauss_point_results\": $PutStrings"
    puts $FileVar "        \},"
    puts $FileVar "        \"point_data_configuration\":  \[\]"
    puts $FileVar "    \},"

    # restart options
    puts $FileVar "    \"restart_options\":     \{"
    puts $FileVar "        \"SaveRestart\":        false,"
    puts $FileVar "        \"RestartFrequency\":   0,"
    puts $FileVar "        \"LoadRestart\":        false,"
    puts $FileVar "        \"Restart_Step\":       0"
    puts $FileVar "    \},"

    # constraint data
    puts $FileVar "    \"constraints_data\":     \{"
    puts $FileVar "        \"incremental_load\":                false,"
    puts $FileVar "        \"incremental_displacement\":        false"
    puts $FileVar "    \}"
    puts $FileVar "\}"

    close $FileVar
}
