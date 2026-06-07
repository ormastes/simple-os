; ModuleID = 'module'
source_filename = "module"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128"
target triple = "riscv64-unknown-unknown"

@simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_BASE = global i64 2149580800
@simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_SIZE = global i64 2097152
@simple_os__arch__riscv64__ghdl_boot_info_entry__MEMORY_MAP_LEN = global i64 3
@simple_os__arch__riscv64__ghdl_boot_info_entry__REGION_KIND_KERNEL = global i64 8
@simple_os__arch__riscv64__ghdl_boot_info_entry__REGION_KIND_RESERVED = global i64 2
@simple_os__arch__riscv64__ghdl_boot_info_entry__REGION_KIND_USABLE = global i64 1

declare i64 @rt_aop_invoke_around(i64, i64, i64, i64, i64)

declare i64 @rt_aop_proceed(i64)

declare i64 @rt_array_new(i64)

declare i64 @rt_array_get(i64, i64)

declare i64 @rt_array_pop(i64)

declare i64 @rt_array_len(i64)

declare i64 @rt_array_first(i64)

declare i64 @rt_array_last(i64)

declare i64 @rt_len(i64)

declare i64 @rt_tuple_new(i64)

declare i64 @rt_tuple_get(i64, i64)

declare i64 @rt_tuple_len(i64)

declare i64 @rt_dict_new(i64)

declare i64 @rt_dict_get(i64, i64)

declare i64 @rt_dict_len(i64)

declare i64 @rt_dict_keys(i64)

declare i64 @rt_dict_values(i64)

declare i64 @rt_index_get(i64, i64)

declare i64 @rt_slice(i64, i64, i64, i64)

declare i64 @rt_vec_sum(i64)

declare i64 @rt_vec_product(i64)

declare i64 @rt_vec_min(i64)

declare i64 @rt_vec_max(i64)

declare i64 @rt_vec_extract(i64, i64)

declare i64 @rt_vec_with(i64, i64, i64)

declare i64 @rt_vec_sqrt(i64)

declare i64 @rt_vec_abs(i64)

declare i64 @rt_vec_floor(i64)

declare i64 @rt_vec_ceil(i64)

declare i64 @rt_vec_round(i64)

declare i64 @rt_vec_shuffle(i64, i64)

declare i64 @rt_vec_blend(i64, i64, i64)

declare i64 @rt_vec_select(i64, i64, i64)

declare i64 @rt_vec_load(i64, i64)

declare i64 @rt_vec_gather(i64, i64)

declare i64 @rt_vec_fma(i64, i64, i64)

declare i64 @rt_vec_recip(i64)

declare i64 @rt_vec_masked_load(i64, i64, i64, i64)

declare i64 @rt_vec_min_vec(i64, i64)

declare i64 @rt_vec_max_vec(i64, i64)

declare i64 @rt_vec_clamp(i64, i64, i64)

declare i64 @rt_neighbor_load(i64, i64)

declare i64 @rt_gpu_atomic_add(i64, i64)

declare i64 @rt_gpu_atomic_sub(i64, i64)

declare i64 @rt_gpu_atomic_min(i64, i64)

declare i64 @rt_gpu_atomic_max(i64, i64)

declare i64 @rt_gpu_atomic_and(i64, i64)

declare i64 @rt_gpu_atomic_or(i64, i64)

declare i64 @rt_gpu_atomic_xor(i64, i64)

declare i64 @rt_gpu_atomic_exchange(i64, i64)

declare i64 @rt_gpu_atomic_cmpxchg(i64, i64, i64)

declare i64 @rt_string_new(i64, i64)

declare i64 @rt_string_concat(i64, i64)

declare i64 @rt_string_starts_with(i64, i64)

declare i64 @rt_string_ends_with(i64, i64)

declare i64 @rt_hash_text(i64)

declare i64 @rt_string_eq(i64, i64)

declare i64 @rt_string_len(i64)

declare i64 @rt_string_data(i64)

declare i64 @rt_string_char_at(i64, i64)

declare i64 @rt_string_split(i64, i64)

declare i64 @rt_string_replace(i64, i64, i64)

declare i64 @rt_string_trim(i64)

declare i64 @rt_string_join(i64, i64)

declare i64 @rt_string_to_upper(i64)

declare i64 @rt_string_to_lower(i64)

declare i64 @rt_string_to_int(i64)

declare i64 @rt_string_index_of(i64, i64)

declare i64 @rt_string_find(i64, i64)

declare i64 @rt_string_rfind(i64, i64)

declare i64 @rt_utf8_count_codepoints(i64)

declare i64 @rt_utf8_find_invalid(i64)

declare i64 @rt_text_count_codepoints(i64)

declare i64 @rt_aes_encrypt_block_with_expanded(i64, i64, i64)

declare i64 @rt_aes_decrypt_block_with_expanded(i64, i64, i64)

declare i64 @rt_to_string(i64)

declare i64 @rt_cstring_to_text(i64)

declare i64 @rt_value_int(i64)

declare i64 @rt_value_nil()

declare i64 @rt_value_as_int(i64)

declare i64 @rt_value_to_string(i64)

declare i64 @rt_value_format_string(i64, i64, i64)

declare i64 @rt_value_eq(i64, i64)

declare i64 @rt_value_compare(i64, i64)

declare i64 @rt_native_eq(i64, i64)

declare i64 @rt_native_neq(i64, i64)

declare i64 @rt_closure_func_ptr(i64)

declare i64 @rt_enum_discriminant(i64)

declare i64 @rt_enum_payload(i64)

declare i64 @rt_unwrap_or_self(i64)

declare i64 @rt_option_map(i64, i64)

declare i64 @rt_unique_new(i64)

declare i64 @rt_unique_get(i64)

declare i64 @rt_unique_set(i64, i64)

declare i64 @rt_unique_needs_trace(i64)

declare i64 @rt_shared_new(i64)

declare i64 @rt_shared_get(i64)

declare i64 @rt_shared_clone(i64)

declare i64 @rt_shared_ref_count(i64)

declare i64 @rt_shared_release(i64)

declare i64 @rt_shared_needs_trace(i64)

declare i64 @rt_shared_downgrade(i64)

declare i64 @rt_weak_new(i64, i64)

declare i64 @rt_weak_upgrade(i64)

declare i64 @rt_weak_is_valid(i64)

declare i64 @rt_handle_new(i64)

declare i64 @rt_handle_get(i64)

declare i64 @rt_handle_set(i64, i64)

declare i64 @rt_handle_is_valid(i64)

declare i64 @rt_alloc(i64)

declare i64 @rt_ptr_to_value(i64)

declare i64 @rt_value_to_ptr(i64)

declare i64 @rt_dyn_torch_tensor_from_bits_1d(i64, i64)

declare i64 @rt_port_inb(i64)

declare i64 @rt_port_outb(i64, i64)

declare i64 @rt_port_inw(i64)

declare i64 @rt_port_outw(i64, i64)

declare i64 @rt_port_inl(i64)

declare i64 @rt_port_outl(i64, i64)

declare i64 @rt_port_io_wait()

declare i64 @rt_cli()

declare i64 @rt_sti()

declare i64 @rt_hlt()

declare i64 @rt_lgdt(i64)

declare i64 @rt_lidt(i64)

declare i64 @rt_ltr(i64)

declare i64 @rt_read_cr2()

declare i64 @rt_read_cr3()

declare i64 @rt_write_cr3(i64)

declare i64 @rt_invlpg(i64)

declare i64 @rt_read_msr(i64)

declare i64 @rt_write_msr(i64, i64)

declare i64 @rt_mmio_read_u8(i64)

declare i64 @rt_mmio_write_u8(i64, i64)

declare i64 @rt_mmio_read_u16(i64)

declare i64 @rt_mmio_write_u16(i64, i64)

declare i64 @rt_mmio_read_u32(i64)

declare i64 @rt_mmio_write_u32(i64, i64)

declare i64 @rt_arm_virtio_blk_mmio_read_u32(i64)

declare i64 @rt_arm_virtio_blk_mmio_read_u64(i64)

declare i64 @rt_arm_virtio_blk_mmio_write_u32(i64, i64)

declare i64 @rt_virtq_desc_write(i64, i64, i64, i64, i64, i64, i64)

declare i64 @rt_dma_bytes_to_array(i64, i64)

declare i64 @rt_bytes_u8_at(i64, i64)

declare i64 @rt_memory_barrier()

declare i64 @rt_arm_virtq_base()

declare i64 @rt_arm_virtio_blk_queue_base()

declare i64 @rt_arm_virtio_blk_dma_base()

declare i64 @rt_arm_virtio_blk_configure_queue(i64)

declare i64 @rt_arm_virtq_used_idx()

declare i64 @rt_arm_virtq_reset()

declare i64 @rt_arm_virtq_push_avail(i64)

declare i64 @rt_arm_virtio_blk_wait_completion(i64)

declare i64 @rt_arm_virtio_blk_status_u8()

declare i64 @rt_arm_virtio_blk_prepare_read(i64)

declare i64 @rt_arm_virtio_blk_read_sector_direct(i64)

declare i64 @rt_arm_virtio_blk_read_prefix(i64, i64)

declare i64 @rt_arm_virtio_blk_read_hello_smf()

declare i64 @rt_arm_virtio_blk_sector_bytes()

declare i64 @rt_array_get_byte_raw(i64, i64)

declare i64 @rt_arm_array_len_u32(i64)

declare i64 @rt_arm_array_get_byte_u32(i64, i64)

declare i64 @rt_arm_array_get_u16_le(i64, i64)

declare i64 @rt_arm_array_get_u32_le(i64, i64)

declare i64 @rt_arm_array_append_bytes(i64, i64, i64)

declare i64 @arm_fs_exec_print_success_marker()

declare i64 @rt_wait(i64)

declare i64 @rt_future_new(i64, i64)

declare i64 @rt_future_await(i64)

declare i64 @rt_future_is_ready(i64)

declare i64 @rt_future_get_result(i64)

declare i64 @rt_future_all(i64)

declare i64 @rt_future_race(i64)

declare i64 @rt_future_resolve(i64)

declare i64 @rt_future_reject(i64)

declare i64 @rt_async_get_state(i64)

declare i64 @rt_async_get_ctx(i64)

declare i64 @rt_actor_spawn(i64, i64)

declare i64 @rt_actor_recv()

declare i64 @rt_actor_join(i64)

declare i64 @rt_actor_reply(i64)

declare i64 @rt_actor_id(i64)

declare i64 @rt_actor_is_alive(i64)

declare i64 @rt_channel_new()

declare i64 @rt_channel_send(i64, i64)

declare i64 @rt_channel_recv(i64)

declare i64 @rt_channel_try_recv(i64)

declare i64 @rt_channel_recv_timeout(i64, i64)

declare i64 @rt_channel_is_closed(i64)

declare i64 @rt_channel_id(i64)

declare i64 @rt_executor_set_mode(i64)

declare i64 @rt_executor_get_mode()

declare i64 @rt_executor_poll()

declare i64 @rt_executor_poll_all()

declare i64 @rt_executor_pending_count()

declare i64 @rt_executor_is_manual()

declare i64 @rt_async_spawn(i64)

declare i64 @rt_async_schedule_await(i64)

declare i64 @rt_async_run_until_complete(i64)

declare i64 @rt_async_spawn_task(i64)

declare i64 @rt_async_poll_tasks()

declare i64 @rt_thread_spawn_isolated(i64, i64)

declare i64 @rt_thread_spawn_isolated_with_args(i64, i64, i64)

declare i64 @rt_thread_join(i64)

declare i64 @rt_thread_is_done(i64)

declare i64 @rt_thread_id(i64)

declare i64 @rt_thread_available_parallelism()

declare i64 @rt_generator_new(i64, i64, i64)

declare i64 @rt_generator_next(i64)

declare i64 @rt_generator_get_state(i64)

declare i64 @rt_generator_load_slot(i64, i64)

declare i64 @rt_generator_get_ctx(i64)

declare i64 @rt_interp_call(i64, i64, i64, i64)

declare i64 @rt_interp_eval(i64)

declare i64 @rt_function_not_found(i64, i64)

declare i64 @rt_method_not_found(i64, i64, i64, i64)

declare i64 @rt_contract_violation_new(i64, i64, i64, i64, i64)

declare i64 @rt_contract_violation_kind(i64)

declare i64 @rt_contract_violation_func_name(i64)

declare i64 @rt_contract_violation_message(i64)

declare i64 @rt_is_contract_violation(i64)

declare i64 @rt_read_stdin_line()

declare i64 @doctest_read_file(i64)

declare i64 @doctest_path_exists(i64)

declare i64 @doctest_is_file(i64)

declare i64 @doctest_is_dir(i64)

declare i64 @doctest_walk_directory(i64, i64, i64)

declare i64 @doctest_path_has_extension(i64, i64)

declare i64 @doctest_path_contains(i64, i64)

declare i64 @rt_gpu_atomic_add_i64(i64, i64)

declare i64 @rt_gpu_atomic_sub_i64(i64, i64)

declare i64 @rt_gpu_atomic_xchg_i64(i64, i64)

declare i64 @rt_gpu_atomic_cmpxchg_i64(i64, i64, i64)

declare i64 @rt_gpu_atomic_min_i64(i64, i64)

declare i64 @rt_gpu_atomic_max_i64(i64, i64)

declare i64 @rt_gpu_atomic_and_i64(i64, i64)

declare i64 @rt_gpu_atomic_or_i64(i64, i64)

declare i64 @rt_gpu_atomic_xor_i64(i64, i64)

declare i64 @rt_gpu_shared_alloc(i64)

declare i64 @rt_vk_device_create()

declare i64 @rt_vk_buffer_alloc(i64, i64)

declare i64 @rt_vk_kernel_compile(i64, i64, i64)

declare i64 @rt_vk_shader_module_create(i64, i64, i64)

declare i64 @rt_vk_image_get_view(i64)

declare i64 @rt_vk_command_buffer_begin(i64)

declare i64 @native_tcp_flush(i64)

declare i64 @native_tcp_shutdown(i64, i64)

declare i64 @native_tcp_close(i64)

declare i64 @native_tcp_set_backlog(i64, i64)

declare i64 @native_tcp_set_nodelay(i64, i64)

declare i64 @native_tcp_set_keepalive(i64, i64)

declare i64 @native_tcp_set_read_timeout(i64, i64)

declare i64 @native_tcp_set_write_timeout(i64, i64)

declare i64 @native_udp_connect(i64, i64, i64)

declare i64 @native_udp_set_broadcast(i64, i64)

declare i64 @native_udp_set_multicast_loop(i64, i64)

declare i64 @native_udp_set_multicast_ttl(i64, i64)

declare i64 @native_udp_set_ttl(i64, i64)

declare i64 @native_udp_set_read_timeout(i64, i64)

declare i64 @native_udp_set_write_timeout(i64, i64)

declare i64 @native_udp_join_multicast_v4(i64, i64, i64)

declare i64 @native_udp_leave_multicast_v4(i64, i64, i64)

declare i64 @native_udp_join_multicast_v6(i64, i64, i64)

declare i64 @native_udp_leave_multicast_v6(i64, i64, i64)

declare i64 @native_udp_close(i64)

declare i64 @rt_coverage_dump_sdn()

declare i64 @rt_perf_clock_ns()

declare i64 @rt_perf_rdtsc()

declare i64 @rt_perf_cycles_to_ns(i64, i64)

declare i64 @rt_perf_dump_sdn()

declare i64 @rt_ffi_type_register(i64, i64, i64)

declare i64 @rt_ffi_new(i64)

declare i64 @rt_ffi_type_id(i64)

declare i64 @rt_ffi_type_name(i64)

declare i64 @rt_ffi_call_method(i64, i64, i64, i64, i64)

declare i64 @rt_ffi_clone(i64)

declare i64 @rt_ffi_object_new(i64, i64, i64)

declare i64 @rt_ffi_object_call_method(i64, i64, i64, i64, i64)

declare i64 @rt_ffi_object_type_id(i64)

declare i64 @rt_ffi_object_type_name(i64)

declare i64 @rt_process_run(i64, i64, i64)

declare i64 @rt_process_spawn(i64, i64, i64)

declare i64 @rt_process_run_timeout(i64, i64, i64, i64)

declare i64 @rt_process_is_running(i64)

declare i64 @rt_process_wait(i64, i64)

declare i64 @rt_process_kill(i64)

declare i64 @rt_process_spawn_async(i64, i64, i64)

declare i64 @rt_process_run_with_limits(i64, i64, i64, i64, i64)

declare i64 @rt_process_exists(i64)

declare i64 @rt_cli_version()

declare i64 @rt_cli_get_args()

declare i64 @rt_cli_read_file(i64)

declare i64 @rt_cli_watch_file(i64)

declare i64 @rt_test_db_validate(i64)

declare i64 @rt_test_db_cleanup_stale_runs(i64)

declare i64 @rt_cli_run_lint(i64)

declare i64 @rt_cli_run_fmt(i64)

declare i64 @rt_cli_run_check(i64)

declare i64 @rt_cli_run_migrate(i64)

declare i64 @rt_cli_run_mcp(i64)

declare i64 @rt_cli_run_diff(i64)

declare i64 @rt_cli_run_context(i64)

declare i64 @rt_cli_run_constr(i64)

declare i64 @rt_cli_run_query(i64)

declare i64 @rt_cli_run_info(i64)

declare i64 @rt_cli_run_spec_coverage(i64)

declare i64 @rt_cli_run_replay(i64)

declare i64 @rt_cli_run_gen_lean(i64)

declare i64 @rt_cli_run_feature_gen(i64)

declare i64 @rt_cli_run_task_gen(i64)

declare i64 @rt_cli_run_spec_gen(i64)

declare i64 @rt_cli_run_spipe_docgen(i64)

declare i64 @rt_cli_run_todo_scan(i64)

declare i64 @rt_cli_run_todo_gen(i64)

declare i64 @rt_cli_run_i18n(i64)

declare i64 @rt_cli_run_lex(i64)

declare i64 @rt_cli_run_brief(i64)

declare i64 @rt_cli_run_ffi_gen(i64)

declare i64 @rt_context_generate(i64, i64, i64)

declare i64 @rt_context_stats(i64, i64)

declare i64 @rt_settlement_main()

declare i64 @rt_native_build(i64)

declare i64 @rt_cli_handle_compile(i64)

declare i64 @rt_cli_handle_targets()

declare i64 @rt_cli_handle_linkers()

declare i64 @rt_cli_handle_web(i64)

declare i64 @rt_cli_handle_diagram(i64)

declare i64 @rt_cli_handle_init(i64)

declare i64 @rt_cli_handle_add(i64)

declare i64 @rt_cli_handle_remove(i64)

declare i64 @rt_cli_handle_install()

declare i64 @rt_cli_handle_update(i64)

declare i64 @rt_cli_handle_list()

declare i64 @rt_cli_handle_tree()

declare i64 @rt_cli_handle_cache(i64)

declare i64 @rt_cli_handle_env(i64)

declare i64 @rt_cli_handle_lock(i64)

declare i64 @rt_sdn_version()

declare i64 @rt_sdn_check(i64)

declare i64 @rt_sdn_to_json(i64)

declare i64 @rt_sdn_from_json(i64)

declare i64 @rt_sdn_get(i64, i64)

declare i64 @rt_sdn_set(i64, i64, i64)

declare i64 @rt_sdn_fmt(i64)

declare i64 @rt_cranelift_module_new(i64, i64)

declare i64 @rt_cranelift_new_module(i64, i64, i64)

declare i64 @rt_cranelift_new_aot_module(i64, i64, i64)

declare i64 @rt_cranelift_finalize_module(i64)

declare i64 @rt_cranelift_new_signature(i64)

declare i64 @rt_cranelift_begin_function(i64, i64, i64, i64)

declare i64 @rt_cranelift_end_function(i64)

declare i64 @rt_cranelift_create_block(i64)

declare i64 @rt_cranelift_iconst(i64, i64, i64)

declare i64 @rt_cranelift_null(i64, i64)

declare i64 @rt_cranelift_iadd(i64, i64, i64)

declare i64 @rt_cranelift_isub(i64, i64, i64)

declare i64 @rt_cranelift_imul(i64, i64, i64)

declare i64 @rt_cranelift_sdiv(i64, i64, i64)

declare i64 @rt_cranelift_udiv(i64, i64, i64)

declare i64 @rt_cranelift_srem(i64, i64, i64)

declare i64 @rt_cranelift_urem(i64, i64, i64)

declare i64 @rt_cranelift_fadd(i64, i64, i64)

declare i64 @rt_cranelift_fsub(i64, i64, i64)

declare i64 @rt_cranelift_fmul(i64, i64, i64)

declare i64 @rt_cranelift_fdiv(i64, i64, i64)

declare i64 @rt_cranelift_band(i64, i64, i64)

declare i64 @rt_cranelift_bor(i64, i64, i64)

declare i64 @rt_cranelift_bxor(i64, i64, i64)

declare i64 @rt_cranelift_bnot(i64, i64)

declare i64 @rt_cranelift_ishl(i64, i64, i64)

declare i64 @rt_cranelift_sshr(i64, i64, i64)

declare i64 @rt_cranelift_ushr(i64, i64, i64)

declare i64 @rt_cranelift_icmp(i64, i64, i64, i64)

declare i64 @rt_cranelift_fcmp(i64, i64, i64, i64)

declare i64 @rt_cranelift_load(i64, i64, i64, i64)

declare i64 @rt_cranelift_stack_slot(i64, i64, i64)

declare i64 @rt_cranelift_stack_addr(i64, i64, i64)

declare i64 @rt_cranelift_call(i64, i64, i64, i64)

declare i64 @rt_cranelift_call_indirect(i64, i64, i64, i64, i64)

declare i64 @rt_cranelift_sextend(i64, i64, i64)

declare i64 @rt_cranelift_uextend(i64, i64, i64)

declare i64 @rt_cranelift_ireduce(i64, i64, i64)

declare i64 @rt_cranelift_fcvt_to_sint(i64, i64, i64)

declare i64 @rt_cranelift_fcvt_to_uint(i64, i64, i64)

declare i64 @rt_cranelift_fcvt_from_sint(i64, i64, i64)

declare i64 @rt_cranelift_fcvt_from_uint(i64, i64, i64)

declare i64 @rt_cranelift_bitcast(i64, i64, i64)

declare i64 @rt_cranelift_append_block_param(i64, i64, i64)

declare i64 @rt_cranelift_block_param(i64, i64, i64)

declare i64 @rt_cranelift_get_function_ptr(i64, i64, i64)

declare i64 @rt_cranelift_call_function_ptr(i64, i64, i64)

declare i64 @rt_file_hash(i64)

declare i64 @rt_env_get(i64, i64)

declare i64 @rt_get_env(i64, i64)

declare i64 @rt_env_cwd()

declare i64 @rt_env_all()

declare i64 @rt_env_vars()

declare i64 @rt_env_home()

declare i64 @rt_env_temp()

declare i64 @rt_get_args()

declare i64 @rt_platform_name()

declare i64 @rt_term_enable_ansi()

declare i64 @rt_term_get_size()

declare i64 @rt_ssh_userauth_password_only_failure_payload()

declare i64 @rt_file_canonicalize(i64, i64)

declare i64 @rt_file_read_text(i64, i64)

declare i64 @rt_file_read_text_rv(i64)

declare i64 @rt_file_size(i64, i64)

declare i64 @rt_file_hash_sha256(i64, i64)

declare i64 @rt_file_read_lines(i64, i64)

declare i64 @rt_file_read_bytes(i64, i64)

declare i64 @rt_bytes_from_raw(i64, i64)

declare i64 @rt_dir_list(i64, i64)

declare i64 @rt_file_find(i64, i64, i64, i64)

declare i64 @rt_dir_glob(i64, i64)

declare i64 @rt_dir_walk(i64, i64)

declare i64 @rt_current_dir()

declare i64 @rt_path_basename(i64, i64)

declare i64 @rt_path_dirname(i64, i64)

declare i64 @rt_path_ext(i64, i64)

declare i64 @rt_path_absolute(i64, i64)

declare i64 @rt_path_separator()

declare i64 @rt_path_stem(i64, i64)

declare i64 @rt_path_relative(i64, i64, i64, i64)

declare i64 @rt_path_join(i64, i64, i64, i64)

declare i64 @ffi_regex_find(i64, i64, i64, i64)

declare i64 @ffi_regex_find_all(i64, i64, i64, i64)

declare i64 @ffi_regex_captures(i64, i64, i64, i64)

declare i64 @ffi_regex_replace(i64, i64, i64, i64, i64, i64)

declare i64 @ffi_regex_replace_all(i64, i64, i64, i64, i64, i64)

declare i64 @ffi_regex_split(i64, i64, i64, i64)

declare i64 @ffi_regex_split_n(i64, i64, i64, i64, i64)

declare i64 @rt_bdd_has_failure()

declare i64 @rt_bdd_format_results()

declare i64 @spl_dlopen(i64)

declare i64 @spl_dlsym(i64, i64)

declare i64 @spl_dlclose(i64)

declare i64 @spl_wffi_call_i64(i64, i64, i64)

define i64 @simple_os__arch__riscv64__ghdl_boot_info_entry___start() {
bb0:
  call void asm sideeffect "mv      s0, a0\0Amv      s1, a1\0Ala      sp, _stack_top\0Amv      a0, s0\0Amv      a1, s1\0Acall    simple_os__arch__riscv64__ghdl_boot_info_entry__boot_main\0Aj       simple_os__arch__riscv64__ghdl_boot_info_entry___halt", ""()
  ret i64 0
}

define i64 @simple_os__arch__riscv64__ghdl_boot_info_entry___halt() {
bb0:
  call void asm sideeffect "ebreak\0A.Lghdl_boot_info_halt:\0Awfi\0Aj .Lghdl_boot_info_halt", ""()
  ret i64 0
}

define i64 @simple_os__arch__riscv64__ghdl_boot_info_entry___record_boot_regions() {
bb0:
  %ram_base = alloca i64, align 8
  %ram_size = alloca i64, align 8
  %usable_base = alloca i64, align 8
  %usable_size = alloca i64, align 8
  %v24 = alloca i64, align 8
  store i64 0, ptr %v24, align 4
  %v12 = alloca i64, align 8
  store i64 0, ptr %v12, align 4
  %v44 = alloca i64, align 8
  store i64 0, ptr %v44, align 4
  %v22 = alloca i64, align 8
  store i64 0, ptr %v22, align 4
  %v18 = alloca i64, align 8
  store i64 0, ptr %v18, align 4
  %v54 = alloca i64, align 8
  store i64 0, ptr %v54, align 4
  %v31 = alloca i64, align 8
  store i64 0, ptr %v31, align 4
  %v14 = alloca i64, align 8
  store i64 0, ptr %v14, align 4
  %v35 = alloca i64, align 8
  store i64 0, ptr %v35, align 4
  %v56 = alloca i64, align 8
  store i64 0, ptr %v56, align 4
  %v27 = alloca i64, align 8
  store i64 0, ptr %v27, align 4
  %v10 = alloca i64, align 8
  store i64 0, ptr %v10, align 4
  %v23 = alloca i64, align 8
  store i64 0, ptr %v23, align 4
  %v42 = alloca i64, align 8
  store i64 0, ptr %v42, align 4
  %v2 = alloca i64, align 8
  store i64 0, ptr %v2, align 4
  %v43 = alloca i64, align 8
  store i64 0, ptr %v43, align 4
  %v52 = alloca i64, align 8
  store i64 0, ptr %v52, align 4
  %v1 = alloca i64, align 8
  store i64 0, ptr %v1, align 4
  %v33 = alloca i64, align 8
  store i64 0, ptr %v33, align 4
  %v57 = alloca i64, align 8
  store i64 0, ptr %v57, align 4
  %v4 = alloca i64, align 8
  store i64 0, ptr %v4, align 4
  %v13 = alloca i64, align 8
  store i64 0, ptr %v13, align 4
  %v51 = alloca i64, align 8
  store i64 0, ptr %v51, align 4
  %v36 = alloca i64, align 8
  store i64 0, ptr %v36, align 4
  %v46 = alloca i64, align 8
  store i64 0, ptr %v46, align 4
  %v48 = alloca i64, align 8
  store i64 0, ptr %v48, align 4
  %v9 = alloca i64, align 8
  store i64 0, ptr %v9, align 4
  %v11 = alloca i64, align 8
  store i64 0, ptr %v11, align 4
  %v58 = alloca i64, align 8
  store i64 0, ptr %v58, align 4
  %v47 = alloca i64, align 8
  store i64 0, ptr %v47, align 4
  %v53 = alloca i64, align 8
  store i64 0, ptr %v53, align 4
  %v6 = alloca i64, align 8
  store i64 0, ptr %v6, align 4
  %v50 = alloca i64, align 8
  store i64 0, ptr %v50, align 4
  %v32 = alloca i64, align 8
  store i64 0, ptr %v32, align 4
  %v21 = alloca i64, align 8
  store i64 0, ptr %v21, align 4
  %v20 = alloca i64, align 8
  store i64 0, ptr %v20, align 4
  %v49 = alloca i64, align 8
  store i64 0, ptr %v49, align 4
  %v0 = alloca i64, align 8
  store i64 0, ptr %v0, align 4
  %v5 = alloca i64, align 8
  store i64 0, ptr %v5, align 4
  %v19 = alloca i64, align 8
  store i64 0, ptr %v19, align 4
  %v38 = alloca i64, align 8
  store i64 0, ptr %v38, align 4
  %v55 = alloca i64, align 8
  store i64 0, ptr %v55, align 4
  %v34 = alloca i64, align 8
  store i64 0, ptr %v34, align 4
  %v45 = alloca i64, align 8
  store i64 0, ptr %v45, align 4
  %v39 = alloca i64, align 8
  store i64 0, ptr %v39, align 4
  %v3 = alloca i64, align 8
  store i64 0, ptr %v3, align 4
  %v41 = alloca i64, align 8
  store i64 0, ptr %v41, align 4
  %v30 = alloca i64, align 8
  store i64 0, ptr %v30, align 4
  %v26 = alloca i64, align 8
  store i64 0, ptr %v26, align 4
  %v28 = alloca i64, align 8
  store i64 0, ptr %v28, align 4
  %v17 = alloca i64, align 8
  store i64 0, ptr %v17, align 4
  %v37 = alloca i64, align 8
  store i64 0, ptr %v37, align 4
  %v40 = alloca i64, align 8
  store i64 0, ptr %v40, align 4
  %v25 = alloca i64, align 8
  store i64 0, ptr %v25, align 4
  %v7 = alloca i64, align 8
  store i64 0, ptr %v7, align 4
  %v8 = alloca i64, align 8
  store i64 0, ptr %v8, align 4
  %v16 = alloca i64, align 8
  store i64 0, ptr %v16, align 4
  %v15 = alloca i64, align 8
  store i64 0, ptr %v15, align 4
  %v29 = alloca i64, align 8
  store i64 0, ptr %v29, align 4
  %call = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_ram_base()
  store i64 %call, ptr %v0, align 4
  %vp2i = ptrtoint ptr %ram_base to i64
  store i64 %vp2i, ptr %v1, align 4
  %v110 = load i64, ptr %v1, align 4
  %v011 = load i64, ptr %v0, align 4
  %store_ptr = inttoptr i64 %v110 to ptr
  store i64 %v011, ptr %store_ptr, align 4
  %call12 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_ram_size()
  store i64 %call12, ptr %v2, align 4
  %vp2i13 = ptrtoint ptr %ram_size to i64
  store i64 %vp2i13, ptr %v3, align 4
  %v314 = load i64, ptr %v3, align 4
  %v215 = load i64, ptr %v2, align 4
  %store_ptr16 = inttoptr i64 %v314 to ptr
  store i64 %v215, ptr %store_ptr16, align 4
  %gload = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_BASE, align 4
  %gload17 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_SIZE, align 4
  %v418 = load i64, ptr %v4, align 4
  %v519 = load i64, ptr %v5, align 4
  %add = add i64 %v418, %v519
  store i64 %add, ptr %v6, align 4
  %vp2i20 = ptrtoint ptr %usable_base to i64
  store i64 %vp2i20, ptr %v7, align 4
  %v721 = load i64, ptr %v7, align 4
  %v622 = load i64, ptr %v6, align 4
  %store_ptr23 = inttoptr i64 %v721 to ptr
  store i64 %v622, ptr %store_ptr23, align 4
  %vp2i24 = ptrtoint ptr %ram_base to i64
  store i64 %vp2i24, ptr %v9, align 4
  %v925 = load i64, ptr %v9, align 4
  %load_ptr = inttoptr i64 %v925 to ptr
  %load = load i64, ptr %load_ptr, align 4
  store i64 %load, ptr %v8, align 4
  %vp2i26 = ptrtoint ptr %ram_size to i64
  store i64 %vp2i26, ptr %v11, align 4
  %v1127 = load i64, ptr %v11, align 4
  %load_ptr28 = inttoptr i64 %v1127 to ptr
  %load29 = load i64, ptr %load_ptr28, align 4
  store i64 %load29, ptr %v10, align 4
  %v830 = load i64, ptr %v8, align 4
  %v1031 = load i64, ptr %v10, align 4
  %add32 = add i64 %v830, %v1031
  store i64 %add32, ptr %v12, align 4
  %vp2i33 = ptrtoint ptr %usable_base to i64
  store i64 %vp2i33, ptr %v14, align 4
  %v1434 = load i64, ptr %v14, align 4
  %load_ptr35 = inttoptr i64 %v1434 to ptr
  %load36 = load i64, ptr %load_ptr35, align 4
  store i64 %load36, ptr %v13, align 4
  %v1237 = load i64, ptr %v12, align 4
  %v1338 = load i64, ptr %v13, align 4
  %sub = sub i64 %v1237, %v1338
  store i64 %sub, ptr %v15, align 4
  %vp2i39 = ptrtoint ptr %usable_size to i64
  store i64 %vp2i39, ptr %v16, align 4
  %v1640 = load i64, ptr %v16, align 4
  %v1541 = load i64, ptr %v15, align 4
  %store_ptr42 = inttoptr i64 %v1640 to ptr
  store i64 %v1541, ptr %store_ptr42, align 4
  store i64 7, ptr %v17, align 4
  %call43 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_uart_base()
  store i64 %call43, ptr %v18, align 4
  %v1744 = load i64, ptr %v17, align 4
  %v1845 = load i64, ptr %v18, align 4
  %call46 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v1744, i64 %v1845)
  store i64 %call46, ptr %v19, align 4
  store i64 8, ptr %v20, align 4
  store i64 1, ptr %v21, align 4
  %v2047 = load i64, ptr %v20, align 4
  %v2148 = load i64, ptr %v21, align 4
  %call49 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_bool(i64 %v2047, i64 %v2148)
  store i64 %call49, ptr %v22, align 4
  store i64 9, ptr %v23, align 4
  %gload50 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__MEMORY_MAP_LEN, align 4
  %v2351 = load i64, ptr %v23, align 4
  %v2452 = load i64, ptr %v24, align 4
  %call53 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v2351, i64 %v2452)
  store i64 %call53, ptr %v25, align 4
  store i64 10, ptr %v26, align 4
  %vp2i54 = ptrtoint ptr %ram_base to i64
  store i64 %vp2i54, ptr %v28, align 4
  %v2855 = load i64, ptr %v28, align 4
  %load_ptr56 = inttoptr i64 %v2855 to ptr
  %load57 = load i64, ptr %load_ptr56, align 4
  store i64 %load57, ptr %v27, align 4
  %v2658 = load i64, ptr %v26, align 4
  %v2759 = load i64, ptr %v27, align 4
  %call60 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v2658, i64 %v2759)
  store i64 %call60, ptr %v29, align 4
  store i64 11, ptr %v30, align 4
  %gload61 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_BASE, align 4
  %vp2i62 = ptrtoint ptr %ram_base to i64
  store i64 %vp2i62, ptr %v33, align 4
  %v3363 = load i64, ptr %v33, align 4
  %load_ptr64 = inttoptr i64 %v3363 to ptr
  %load65 = load i64, ptr %load_ptr64, align 4
  store i64 %load65, ptr %v32, align 4
  %v3166 = load i64, ptr %v31, align 4
  %v3267 = load i64, ptr %v32, align 4
  %sub68 = sub i64 %v3166, %v3267
  store i64 %sub68, ptr %v34, align 4
  %v3069 = load i64, ptr %v30, align 4
  %v3470 = load i64, ptr %v34, align 4
  %call71 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v3069, i64 %v3470)
  store i64 %call71, ptr %v35, align 4
  store i64 12, ptr %v36, align 4
  %gload72 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__REGION_KIND_RESERVED, align 4
  %v3673 = load i64, ptr %v36, align 4
  %v3774 = load i64, ptr %v37, align 4
  %call75 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v3673, i64 %v3774)
  store i64 %call75, ptr %v38, align 4
  store i64 13, ptr %v39, align 4
  %gload76 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_BASE, align 4
  %v3977 = load i64, ptr %v39, align 4
  %v4078 = load i64, ptr %v40, align 4
  %call79 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v3977, i64 %v4078)
  store i64 %call79, ptr %v41, align 4
  store i64 14, ptr %v42, align 4
  %gload80 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__KERNEL_LOAD_SIZE, align 4
  %v4281 = load i64, ptr %v42, align 4
  %v4382 = load i64, ptr %v43, align 4
  %call83 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v4281, i64 %v4382)
  store i64 %call83, ptr %v44, align 4
  store i64 15, ptr %v45, align 4
  %gload84 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__REGION_KIND_KERNEL, align 4
  %v4585 = load i64, ptr %v45, align 4
  %v4686 = load i64, ptr %v46, align 4
  %call87 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v4585, i64 %v4686)
  store i64 %call87, ptr %v47, align 4
  store i64 16, ptr %v48, align 4
  %vp2i88 = ptrtoint ptr %usable_base to i64
  store i64 %vp2i88, ptr %v50, align 4
  %v5089 = load i64, ptr %v50, align 4
  %load_ptr90 = inttoptr i64 %v5089 to ptr
  %load91 = load i64, ptr %load_ptr90, align 4
  store i64 %load91, ptr %v49, align 4
  %v4892 = load i64, ptr %v48, align 4
  %v4993 = load i64, ptr %v49, align 4
  %call94 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v4892, i64 %v4993)
  store i64 %call94, ptr %v51, align 4
  store i64 17, ptr %v52, align 4
  %vp2i95 = ptrtoint ptr %usable_size to i64
  store i64 %vp2i95, ptr %v54, align 4
  %v5496 = load i64, ptr %v54, align 4
  %load_ptr97 = inttoptr i64 %v5496 to ptr
  %load98 = load i64, ptr %load_ptr97, align 4
  store i64 %load98, ptr %v53, align 4
  %v5299 = load i64, ptr %v52, align 4
  %v53100 = load i64, ptr %v53, align 4
  %call101 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v5299, i64 %v53100)
  store i64 %call101, ptr %v55, align 4
  store i64 18, ptr %v56, align 4
  %gload102 = load i64, ptr @simple_os__arch__riscv64__ghdl_boot_info_entry__REGION_KIND_USABLE, align 4
  %v56103 = load i64, ptr %v56, align 4
  %v57104 = load i64, ptr %v57, align 4
  %call105 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v56103, i64 %v57104)
  store i64 %call105, ptr %v58, align 4
  ret i64 0
}

define i64 @simple_os__arch__riscv64__ghdl_boot_info_entry__boot_main(i64 %0, i64 %1) {
bb0:
  %hart_id = alloca i64, align 8
  %dtb_addr = alloca i64, align 8
  %_boot_info = alloca i64, align 8
  %v3 = alloca i64, align 8
  store i64 0, ptr %v3, align 4
  %v5 = alloca i64, align 8
  store i64 0, ptr %v5, align 4
  %v23 = alloca i64, align 8
  store i64 0, ptr %v23, align 4
  %v14 = alloca i64, align 8
  store i64 0, ptr %v14, align 4
  %v22 = alloca i64, align 8
  store i64 0, ptr %v22, align 4
  %v26 = alloca i64, align 8
  store i64 0, ptr %v26, align 4
  %v8 = alloca i64, align 8
  store i64 0, ptr %v8, align 4
  %v13 = alloca i64, align 8
  store i64 0, ptr %v13, align 4
  %v9 = alloca i64, align 8
  store i64 0, ptr %v9, align 4
  %v16 = alloca i64, align 8
  store i64 0, ptr %v16, align 4
  %v10 = alloca i64, align 8
  store i64 0, ptr %v10, align 4
  %v33 = alloca i64, align 8
  store i64 0, ptr %v33, align 4
  %v11 = alloca i64, align 8
  store i64 0, ptr %v11, align 4
  %v29 = alloca i64, align 8
  store i64 0, ptr %v29, align 4
  %v21 = alloca i64, align 8
  store i64 0, ptr %v21, align 4
  %v27 = alloca i64, align 8
  store i64 0, ptr %v27, align 4
  %v25 = alloca i64, align 8
  store i64 0, ptr %v25, align 4
  %v18 = alloca i64, align 8
  store i64 0, ptr %v18, align 4
  %v4 = alloca i64, align 8
  store i64 0, ptr %v4, align 4
  %v6 = alloca i64, align 8
  store i64 0, ptr %v6, align 4
  %v20 = alloca i64, align 8
  store i64 0, ptr %v20, align 4
  %v19 = alloca i64, align 8
  store i64 0, ptr %v19, align 4
  %v24 = alloca i64, align 8
  store i64 0, ptr %v24, align 4
  %v7 = alloca i64, align 8
  store i64 0, ptr %v7, align 4
  %v2 = alloca i64, align 8
  store i64 0, ptr %v2, align 4
  %v31 = alloca i64, align 8
  store i64 0, ptr %v31, align 4
  %v12 = alloca i64, align 8
  store i64 0, ptr %v12, align 4
  %v15 = alloca i64, align 8
  store i64 0, ptr %v15, align 4
  %v1 = alloca i64, align 8
  store i64 0, ptr %v1, align 4
  %v0 = alloca i64, align 8
  store i64 0, ptr %v0, align 4
  %v30 = alloca i64, align 8
  store i64 0, ptr %v30, align 4
  %v32 = alloca i64, align 8
  store i64 0, ptr %v32, align 4
  %v17 = alloca i64, align 8
  store i64 0, ptr %v17, align 4
  %v28 = alloca i64, align 8
  store i64 0, ptr %v28, align 4
  store i64 %0, ptr %hart_id, align 4
  store i64 %0, ptr %v0, align 4
  store i64 %1, ptr %dtb_addr, align 4
  store i64 %1, ptr %v1, align 4
  %v01 = load i64, ptr %v0, align 4
  %v110 = load i64, ptr %v1, align 4
  store i64 0, ptr %v0, align 4
  store i64 4776436086200028998, ptr %v1, align 4
  %v011 = load i64, ptr %v0, align 4
  %v112 = load i64, ptr %v1, align 4
  %call = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v011, i64 %v112)
  store i64 %call, ptr %v2, align 4
  store i64 1, ptr %v3, align 4
  store i64 0, ptr %v4, align 4
  %v313 = load i64, ptr %v3, align 4
  %v414 = load i64, ptr %v4, align 4
  %call15 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v313, i64 %v414)
  store i64 %call15, ptr %v5, align 4
  %vp2i = ptrtoint ptr %hart_id to i64
  store i64 %vp2i, ptr %v7, align 4
  %v716 = load i64, ptr %v7, align 4
  %load_ptr = inttoptr i64 %v716 to ptr
  %load = load i64, ptr %load_ptr, align 4
  store i64 %load, ptr %v6, align 4
  %vp2i17 = ptrtoint ptr %dtb_addr to i64
  store i64 %vp2i17, ptr %v9, align 4
  %v918 = load i64, ptr %v9, align 4
  %load_ptr19 = inttoptr i64 %v918 to ptr
  %load20 = load i64, ptr %load_ptr19, align 4
  store i64 %load20, ptr %v8, align 4
  %v621 = load i64, ptr %v6, align 4
  %v822 = load i64, ptr %v8, align 4
  %call23 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_save_boot_args(i64 %v621, i64 %v822)
  store i64 %call23, ptr %v10, align 4
  %call24 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_parse_boot_info()
  store i64 %call24, ptr %v11, align 4
  %vp2i25 = ptrtoint ptr %_boot_info to i64
  store i64 %vp2i25, ptr %v12, align 4
  %v1226 = load i64, ptr %v12, align 4
  %v1127 = load i64, ptr %v11, align 4
  %store_ptr = inttoptr i64 %v1226 to ptr
  store i64 %v1127, ptr %store_ptr, align 4
  store i64 2, ptr %v13, align 4
  %call28 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_hart_id()
  store i64 %call28, ptr %v14, align 4
  %v1329 = load i64, ptr %v13, align 4
  %v1430 = load i64, ptr %v14, align 4
  %call31 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v1329, i64 %v1430)
  store i64 %call31, ptr %v15, align 4
  store i64 3, ptr %v16, align 4
  %call32 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_dtb_address()
  store i64 %call32, ptr %v17, align 4
  %v1633 = load i64, ptr %v16, align 4
  %v1734 = load i64, ptr %v17, align 4
  %call35 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v1633, i64 %v1734)
  store i64 %call35, ptr %v18, align 4
  store i64 4, ptr %v19, align 4
  %call36 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_dtb_valid()
  store i64 %call36, ptr %v20, align 4
  %v1937 = load i64, ptr %v19, align 4
  %v2038 = load i64, ptr %v20, align 4
  %call39 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_bool(i64 %v1937, i64 %v2038)
  store i64 %call39, ptr %v21, align 4
  store i64 5, ptr %v22, align 4
  %call40 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_ram_base()
  store i64 %call40, ptr %v23, align 4
  %v2241 = load i64, ptr %v22, align 4
  %v2342 = load i64, ptr %v23, align 4
  %call43 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v2241, i64 %v2342)
  store i64 %call43, ptr %v24, align 4
  store i64 6, ptr %v25, align 4
  %call44 = call i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_ram_size()
  store i64 %call44, ptr %v26, align 4
  %v2545 = load i64, ptr %v25, align 4
  %v2646 = load i64, ptr %v26, align 4
  %call47 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v2545, i64 %v2646)
  store i64 %call47, ptr %v27, align 4
  %call48 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_entry___record_boot_regions()
  store i64 %call48, ptr %v28, align 4
  %call49 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_emit_marker()
  store i64 %call49, ptr %v29, align 4
  store i64 1, ptr %v30, align 4
  store i64 1, ptr %v31, align 4
  %v3050 = load i64, ptr %v30, align 4
  %v3151 = load i64, ptr %v31, align 4
  %call52 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64 %v3050, i64 %v3151)
  store i64 %call52, ptr %v32, align 4
  %call53 = call i64 @simple_os__arch__riscv64__ghdl_boot_info_entry___halt()
  store i64 %call53, ptr %v33, align 4
  ret i64 0
}

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_ram_base()

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_ram_size()

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_uart_base()

declare i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_u64(i64, i64)

declare i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_store_bool(i64, i64)

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_save_boot_args(i64, i64)

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_parse_boot_info()

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_hart_id()

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_dtb_address()

declare i64 @os__kernel__arch__riscv64__boot_info__rv64_boot_dtb_valid()

declare i64 @simple_os__arch__riscv64__ghdl_boot_info_proof_layout__proof_emit_marker()
