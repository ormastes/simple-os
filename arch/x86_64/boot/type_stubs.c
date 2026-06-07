/*
 * SimpleOS x86_64 Baremetal Type Constructor Stubs
 *
 * Provides type constructors, module initializers, _dot_ method stubs,
 * and host-only function stubs for baremetal SimpleOS linking.
 *
 * Generated from /tmp/was_undefined.txt symbol list.
 *
 * Sections:
 *   1. Types and macros
 *   2. Module init functions (__init__ symbols) - no-ops
 *   3. Type constructor functions - allocate zeroed struct, return ENCODE_PTR
 *   4. Module-qualified type constructors (double-underscore paths)
 *   5. _dot_ method stubs
 *   6. Collection runtime stubs (__rt_btreemap, __rt_hashmap, etc.)
 *   7. Host-only function stubs (networking, async, logging, etc.)
 *   8. Miscellaneous utility stubs
 */

/* ===================================================================
 * 1. Types and macros
 * =================================================================== */

#include <stdint.h>
#include <stddef.h>

typedef int64_t RuntimeValue;

#define NIL_VALUE       ((RuntimeValue)0x3)
#define ENCODE_INT(v)   ((RuntimeValue)(((uint64_t)(int64_t)(v) << 3)))
#define DECODE_INT(v)   ((int64_t)((uint64_t)(v) >> 3))
#define ENCODE_PTR(p)   ((RuntimeValue)((uint64_t)(uintptr_t)(p) | 1))
#define DECODE_PTR(v)   ((void *)((uint64_t)(v) & ~(uint64_t)7))

extern void *malloc(size_t);

/* Struct allocation helper - allocate zeroed struct, return as tagged pointer */
static inline RuntimeValue alloc_struct(size_t size) {
    void *p = malloc(size);
    if (!p) return NIL_VALUE;
    /* Zero-initialize */
    uint8_t *bp = (uint8_t *)p;
    for (size_t i = 0; i < size; i++) bp[i] = 0;
    return ENCODE_PTR(p);
}

/* ===================================================================
 * 2. Module init functions (__init__ symbols) - 145 total
 *
 * These initialize module-level state. On baremetal, they are no-ops.
 * =================================================================== */

RuntimeValue common____init____AbiVersion(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____ConfigEnv(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____EasyFix(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____FileReader(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____FixReport(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____Label(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____MappedFile(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____Replacement(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____Span(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____Target(void) {
    return NIL_VALUE;
}
RuntimeValue common____init____TargetConfig(void) {
    return NIL_VALUE;
}
RuntimeValue common__pure__nn__init__PureTensor(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____Label(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____LevelConfig(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____Report(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____ReportCollector(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____ReportConfig(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____SourceLocation(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____SourceRegistry(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____Span(void) {
    return NIL_VALUE;
}
RuntimeValue common__report____init____Suggestion(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____Lexer(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____NoOpHandler(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____RestrictedHandler(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____SdnDocument(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____SdnParser(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____Span(void) {
    return NIL_VALUE;
}
RuntimeValue common__sdn____init____Token(void) {
    return NIL_VALUE;
}
RuntimeValue common__tooling__easy_fix____init____EasyFix(void) {
    return NIL_VALUE;
}
RuntimeValue common__tooling__easy_fix____init____FixApplicator(void) {
    return NIL_VALUE;
}
RuntimeValue common__tooling__easy_fix____init____FixReport(void) {
    return NIL_VALUE;
}
RuntimeValue common__tooling__easy_fix____init____Lint(void) {
    return NIL_VALUE;
}
RuntimeValue common__tooling__easy_fix____init____LintResult(void) {
    return NIL_VALUE;
}
RuntimeValue common__tooling__easy_fix____init____Replacement(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____CallGraph(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____ConstKeyValidation(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____EffectEnv(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____FunctionType(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____MixinInfo(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____PromiseTypeInfo(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____RequiredMethodSig(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____SccResult(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____Substitution(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____TypeChecker(void) {
    return NIL_VALUE;
}
RuntimeValue common__type____init____TypeScheme(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_immut__atom____init____AtomicI64(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_immut__persistent_set____init____PersistentMap(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_immut__ref____init____Atom(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____Context(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____Gpu(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuArray(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuFramebuffer(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuGraphicsPipeline(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuPipelineDesc(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuRenderPass(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuRenderPassDesc(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuShader(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuSwapchain(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuSwapchainDesc(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuTexture(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuTextureDesc(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuVertexAttribute(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____GpuVertexLayout(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__gpu____init____List(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____AsyncRouter(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____HandlerRegistry(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____HttpRequestData(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____HttpResponseData(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____HttpServer(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____LocationConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____MiddlewareEntry(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____MiddlewarePipeline(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____ServerConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____StaticFileHandler(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____UpstreamConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____UpstreamServer(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__http_server____init____WorkerHandle(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__llm____init____GeminiApiResponse(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_async_mut__llm____init____LLMResponse(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____AutoImport(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____CyclicDependencyError(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____DirManifest(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____ImportEdge(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____ImportGraph(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____MacroExports(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____MacroSymbol(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____ModDecl(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____ModPath(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____ModuleContents(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____ProjectSymbols(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____Segment(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____Symbol(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____SymbolConflictError(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____SymbolEntry(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____SymbolId(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__dependency_tracker____init____SymbolTable(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____CircuitBreaker(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____CircuitBreakerConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____CircuitBreakerRegistry(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____CircuitBreakerStats(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____ConsoleLogger(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____Counter(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____DapFailSafeConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____Deadline(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____FailSafeContext(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____FailSafeError(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____Gauge(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____LspFailSafeConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____McpFailSafeConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____MetricsRegistry(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____PanicHandler(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____PanicInfo(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____RateLimitConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____RateLimitStats(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____RateLimiter(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____ResourceAlert(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____ResourceLimits(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____ResourceMonitor(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____ResourceMonitorStats(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____ResourceUsage(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____TimeoutConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____TimeoutManager(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____TimeoutStats(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____TimeoutToken(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__failsafe____init____TokenBucket(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__gpu_driver____init____GpuContext(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__gpu_driver____init____GpuDevice(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__gpu_driver____init____GpuError(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__gpu_driver____init____GpuLaunchConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__gpu_driver____init____GpuModule(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__gpu_driver____init____GpuPtr(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____Breakpoint(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____BreakpointLocation(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____DapFailSafeContext(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____DapHandler(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____DapSessionState(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____EvaluateResult(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____FailSafeDapServer(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____Scope(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____StackFrame(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____Thread(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__mcp____init____Variable(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__package__installer____init____InstallerConfig(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__package__installer____init____InstallerResult(void) {
    return NIL_VALUE;
}
RuntimeValue nogc_sync_mut__package__installer____init____ToolAvailability(void) {
    return NIL_VALUE;
}

/* ===================================================================
 * 3. Type constructor functions - 3 total
 *
 * Allocate a zeroed 128-byte struct and return as ENCODE_PTR.
 * 128 bytes is sufficient for most Simple structs (up to 16 fields).
 * =================================================================== */

RuntimeValue Pair(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue PhysMemManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue Slice(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

/* --- Utility functions (not constructors) --- */

RuntimeValue floor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* floor of integer is itself */
}

RuntimeValue i64_to_ptr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    /* Convert integer to pointer - extract int value and encode as ptr */
    int64_t v = DECODE_INT(a);
    return ENCODE_PTR((void *)(uintptr_t)v);
}

RuntimeValue type_id_of(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* Unknown type */
}

RuntimeValue file_metadata(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* No filesystem metadata on baremetal */
}

RuntimeValue path_components(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* No path parsing on baremetal */
}

RuntimeValue task_alloc_id(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    static int64_t next_id = 1;
    return ENCODE_INT(next_id++);
}

/* Architecture-specific stubs */

RuntimeValue _get_kernel_end(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    extern char _kernel_end[];
    return ENCODE_INT((int64_t)(uintptr_t)&_kernel_end);
}

RuntimeValue _get_kernel_start(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    extern char _kernel_start[];
    return ENCODE_INT((int64_t)(uintptr_t)&_kernel_start);
}

RuntimeValue _rv64_trap_vector(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* RISC-V only, not used on x86_64 */
}

/* ===================================================================
 * 4. Module-qualified type constructors - 1810 total
 *
 * Symbols like apps__calculator__calculator__UISession are constructors
 * for types imported from other modules. Allocate zeroed struct.
 * =================================================================== */

RuntimeValue PersistentMapBuilder__new(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue Users__ormastes__simple__examples__simple_os__arch__x86_64__gui_entry__BootMemoryRegion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue Users__ormastes__simple__examples__simple_os__arch__x86_64__gui_entry__BootOutputPort(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue Users__ormastes__simple__examples__simple_os__arch__x86_64__gui_entry__PhysAddr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue Users__ormastes__simple__examples__simple_os__arch__x86_64__gui_entry__VirtAddr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__BeRenderResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__BrowserRenderer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__SceneCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_demo__browser_demo__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__BeLayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__BeRenderResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__BrowserRenderer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__PaintCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__browser_sample__browser_sample__SceneCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__calculator__calculator__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__calculator__calculator__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__calculator__calculator__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__calendar__calendar__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__calendar__calendar__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__calendar__calendar__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__clock__clock__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__clock__clock__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__clock__clock__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__disk_manager__disk_manager__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__disk_manager__disk_manager__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__disk_manager__disk_manager__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__editor__editor__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__editor__editor__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__editor__editor__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__file_explorer__file_explorer__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__file_explorer__file_explorer__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__file_explorer__file_explorer__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__file_manager__file_manager__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__file_manager__file_manager__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__file_manager__file_manager__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__hello_world__hello_world__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__hello_world__hello_world__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__hello_world__hello_world__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__image_viewer__image_viewer__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__image_viewer__image_viewer__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__image_viewer__image_viewer__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__memo__memo__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__memo__memo__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__memo__memo__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__paint__paint__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__paint__paint__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__paint__paint__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__screenshot__screenshot__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__screenshot__screenshot__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__screenshot__screenshot__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__settings__settings_app__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__settings__settings_app__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__settings__settings_app__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__shell__shell_app__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__shell__shell_app__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__shell__shell_app__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__system_monitor__system_monitor__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__system_monitor__system_monitor__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__system_monitor__system_monitor__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__terminal__terminal__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__terminal__terminal__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue apps__terminal__terminal__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue async__future__Future(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__base64__encode__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__CudaFunc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__CudaLaunchConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__CudaModule(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__GpuBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanCommandBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanDescriptorSet(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanDeviceInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanDispatchConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanFramebuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanGraphicsPipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanImage(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanPipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanRenderPass(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanSampler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanShader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__kernels__VulkanSwapchain(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__memory__CudaPtr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__memory__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__memory__VulkanBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__GpuBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__GpuDeviceEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__GpuEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__GpuKFunc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__GpuMemoryPool(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__GpuStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__mod__KernelLaunch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__sync__CudaStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__compute__sync__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__csv__mod__CsvConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__csv__mod__CsvParseResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__csv__mod__CsvTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__easy_fix_rules__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__easy_fix_rules__Replacement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__easy_fix_rules__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__color__RGBA8(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__math2d__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__math3d__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__rect__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__signal__event_bus__Signal(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__signal__event_bus__SignalConnectionId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__signal__signal__SignalConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__engine__signal__signal__SignalConnectionId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__error_format__ErrorBase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fix_applicator__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fix_applicator__Replacement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fix_applicator__SourceRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fix_applicator__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__format_utils__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fsm__mod__FSM(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fsm__mod__FSMState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__fsm__mod__FSMTransition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__io__async_traits__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__io__traits__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__json__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__json__mod__JsonArrayBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__json__mod__JsonBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__json__serializer__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__layout__flex__BoxModel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__layout__flex__LayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__layout__grid__BoxModel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__layout__grid__LayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__CompressionContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__HashTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__LZ77Stats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__LZ77Token(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__LookaheadBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__MatchResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__lz77__mod__SlidingWindow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__math3d__mat__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__math3d__quat__Mat4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__math3d__quat__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__math3d__transform__Mat4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__math3d__transform__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__math3d__transform__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__mesh__mesh__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__mesh__mesh__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__parser__parser__Module(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__parser__parser__Token(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__parser__parser_expr__ParseError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__parser__parser_expr__Parser(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__parser__parser_expr__Token(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__autograd__TensorF64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__autograd_advanced__Tensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__autograd_advanced__TensorF64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__dataloader__ArrayDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__dataloader__LabeledDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__ArrayDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__DataLoader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__DataLoaderIterator(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__Dataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__LabeledBatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__LabeledDataLoader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__LabeledDataLoaderIterator(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__LabeledDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__data__mod__LabeledSample(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__demo__LinearModel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__demo__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__evaluator__Module(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__evaluator_broadcast__EvalError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__evaluator_broadcast__Module(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__metrics__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__mod__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__AvgPool2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__BatchNorm1d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__Embedding(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__LayerNorm(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__MaxPool2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__attention__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__embedding__Tensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__embedding__TensorF64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__functional__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__loss__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__AttentionOutput(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__AvgPool2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__BatchNorm2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__GroupNorm(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__InstanceNorm2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__MaxPool2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__mod__MultiHeadAttention(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__norm__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__pooling__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn__serialization__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__nn_layers__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__optim__mod__CyclicLR(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__optim__mod__LinearLR(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__optim__mod__OneCycleLR(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__optim__mod__ReduceLROnPlateau(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__optim__mod__WarmupLR(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__parser__Module(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__parser__Token(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__parser_expr__Module(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__parser_expr__ParseError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__parser_expr__Parser(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__parser_expr__Token(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__tensor_f64_advanced__TensorF64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__tensor_factory__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__tensor_ops__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__torch_ffi__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__training__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__pure__utils__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__match__MatchResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__match__RegexPattern(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__parse__CharClass(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__replace__MatchResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__replace__RegexPattern(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__utilities__MatchResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex__utilities__RegexPattern(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__ast__RegexAST(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__dfa__DFA(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__dfa__DFAState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__dfa__DFATransition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__matcher__MatchResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__nfa__NFA(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__nfa__NFAState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__nfa__NFATransition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__regex_engine__tokenizer__RegexToken(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__render_scene__executor__DamageRect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__render_scene__executor__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__render_scene__executor__SceneCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__editor__RichDocument(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__editor__TextSpan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__serialize__RichDocument(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__serialize__TextSpan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__to_html__RichDocument(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__to_html__TextSpan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__to_markdown__RichDocument(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__rich_text__to_markdown__TextSpan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__document__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__error__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__handler__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__lexer__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__parser__Lexer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__parser__Span(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__sdn__parser__Token(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__audit_advice__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__audit_advice__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__audit_advice__JoinPointContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__audit_advice__SecurityContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__auth_advice__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__auth_advice__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__auth_advice__JoinPointContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__auth_advice__SecurityContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__validation_advice__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__validation_advice__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__security__aspects__validation_advice__JoinPointContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__text_advanced__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_compiler__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_compiler__Replacement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_helpers__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_lint__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_lint__Replacement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_structural__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__tooling__easy_fix__rules_structural__Replacement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__torch__dyn_ffi__DynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__torch__dyn_ffi__DynLoader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__type__checker__FunctionType(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__type__checker__MixinInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__type__checker__Substitution(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__type__checker__TypeScheme(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__app__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_backend__UIEventBus(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_backend__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_diff__LifecycleRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_diff__UIPatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_diff__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_diff__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_diff__WidgetStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_loop__Channel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_loop__HostRuntime(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_loop__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_reactive__Channel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_reactive__ReactiveStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_reactive__ReactiveValue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_state__UIEventBus(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_state__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__async_state__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__backend__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__backend__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__backend_factory__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__backend_factory__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__builder__LayoutProfile(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__builder__ProfileSet(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__builder__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__builder__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__builder__WidgetProp(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__capability_config__CapabilityPolicy(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__capability_policy__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__capability_policy__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__capability_policy__NotSupported(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__diff__UIPatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__diff__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__diff__WidgetStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__event__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__event__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__event__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_css__GlassDesignTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_theme__BorderStyle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_theme__FontStyle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_theme__GlassDesignTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_theme__Spacing(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_theme__ThemeColors(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_theme__UITheme(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_tokens__IOSAnimationTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_tokens__IOSOpacityTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_tokens__IOSRadiusTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_tokens__IOSShadowTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_tokens__IOSSpacingScale(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__glass_tokens__IOSTypographyScale(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_builder__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_css__IOSDesignTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_css__IOSTextStyle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_theme__BorderStyle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_theme__FontStyle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_theme__IOSDesignTokens(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_theme__Spacing(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_theme__ThemeColors(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__ios_theme__UITheme(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__layout__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__layout__WidgetRect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__lifecycle__UIPatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__lifecycle__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__lifecycle__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__lifecycle__WidgetProp(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__mode__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__mode__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__parse__html__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__parse__html__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__parse__sdn_tree__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__parse__sdn_tree__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__patch__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__profile__Viewport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__profile__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__reactive__UIPatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__reactive__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__reactive__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__reactive__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__CapabilityPolicy(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__ChangeLog(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__LayoutProfile(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__LifecycleRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__ProfileResolver(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__ProfileSet(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__ProfileSetEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__SurfaceHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__SurfaceManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__Viewport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__session__WidgetStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__state__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__state__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__style__Color(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__surface__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__surface__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__widget__WidgetStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__widget_store__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__widget_store__WidgetProp(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__ui__widget_store__WidgetRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__event__WebElement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__mod__WebDocumentHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__mod__WebElement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__mod__WebEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__mod__WebStyle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__query__WebElement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue common__web__style__WebElement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__BeLayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__BorderProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__BoxEdges(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__CssValue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__PaintCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__StyleProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__browser_backend__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__compositor__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__compositor__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__compositor__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__compositor__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__compositor__WmInputEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__fb_backend__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__fb_backend__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__fb_backend__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue compositor__fb_backend__WidgetRect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__atomic__FileLock(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__bug__Bug(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__bug__BugDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__bug__BugStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__checker__DbCheckReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__checker__DbFixResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__checker__DbIssue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__core__SdnDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__core__SdnRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__core__SdnTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__core__StringId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__core__StringInterner(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__feature__CategoryStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__feature__Feature(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__feature__FeatureDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__query__Filter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__query__QueryBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__stats__Stats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__task__TaskDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__task__TaskEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test__TestDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test__TestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test__TestRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__TestInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__test_extended__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__todo__TodoDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue database__todo__TodoEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue db_atomic__AtomicTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue db_atomic__DbConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue db_atomic__FileLock(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue desktop__shell__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue desktop__shell__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue desktop__shell__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue desktop__shell__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__BeLayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__PaintCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__SceneCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__browser_renderer__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__layout__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__layout__BoxEdges(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__layout__CssValue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__layout__StyleProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__BeLayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__BeRenderResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__BorderProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__BoxEdges(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__BrowserRenderer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__CssValue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__PaintCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__mod__StyleProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__paint__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__paint__BeLayoutBox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__paint__BorderProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__paint__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__paint__SceneCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__paint__StyleProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__BeDomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__BorderProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__BoxEdges(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__CssValue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__StyleProps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__browser_engine__widget_to_dom__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuArray(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuFramebuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuGraphicsPipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuPipelineDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuRenderPass(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuRenderPassDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuSwapchain(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuSwapchainDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuTexture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__context__GpuTextureDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__engine__BaremetalBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__engine__CpuBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__engine__OpenGLBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__engine__SoftwareBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__engine__VulkanBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__BaremetalBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__Compositor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__CpuBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__Engine2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__Layer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__OpenGLBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__SoftwareBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__engine2d__mod__VulkanBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__mod__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__gpu__mod__GpuArray(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__Adam(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__CrossEntropyLoss(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__MSELoss(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__RMSprop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__SGD(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__Sequential(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__mod__Stream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__torch_training__Conv2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__torch_training__Linear(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue gc_async_mut__torch__torch_training__Tensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue io_runtime__ShellResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue lib__gpu_bridge__scene_to_gpu__RenderScene(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue lib__gpu_bridge__scene_to_gpu__SceneCommand(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_immut__atom__cas__AtomicI64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__actor__ActorMailbox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__actor__ActorMessage(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__mailbox__MutexHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__spawn__Actor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__spawn__ActorMailbox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__spawn__ActorMessage(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor__spawn__HandlerTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__actor_scheduler__Mailbox(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__AsyncIO(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__Executor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__Promise(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__Scheduler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__Task(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__combinators__Task(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__executor__Task(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__scheduler__Executor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async__scheduler__Task(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__CancellationToken(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostFuturesUnordered(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostJoinSet(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostPromise(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostRuntime(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostScheduler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostTask(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__HostTaskHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__WorkStealingQueue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__combinators__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__future__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__future__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__handle__CancellationToken(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__handle__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__joinset__CancellationToken(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__joinset__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__joinset__HostTaskHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__promise__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__runtime__CancellationToken(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__runtime__HostJoinSet(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__runtime__HostScheduler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__runtime__HostTaskHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__scheduler__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__scheduler__ThreadSafeQueue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__scheduler__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__thread_safe_queue__CondVarHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__thread_safe_queue__MutexHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__unordered__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_host__worker_thread__ThreadSafeQueue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostFuturesUnordered(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostJoinSet(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostPromise(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostRuntime(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostScheduler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostTask(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__HostTaskHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__ThreadSafeQueue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__WorkStealingQueue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__async_unified__WorkerThread(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__gpu__context__GpuArray(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__gpu__context__Stream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__gpu__device__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__gpu__mod__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__gpu__mod__GpuArray(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__access_log__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__access_log__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__config__LocationConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__config__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__config__UpstreamConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__config__UpstreamServer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__connection__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__connection__HttpRequestParser(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__connection__HttpResponseData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__connection__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__cors__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__cors__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__csrf__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__csrf__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__csrf__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__csrf__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__handler__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__handler__HttpResponseData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__handler__LocationConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__handler__StaticFileHandler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__middleware__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__middleware__HttpResponseData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__middleware__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__parser__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__rate_limit__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__rate_limit__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__rate_limit__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__rate_limit__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__request_validation__AuditConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__request_validation__AuditEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__request_validation__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__request_validation__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__response__HttpResponseData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__router__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__router__LocationConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__security_headers__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__security_headers__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__AsyncRouter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__Channel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__ChildSpec(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__HandlerRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__MiddlewarePipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__Supervisor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__SupervisorConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__Worker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__server__WorkerHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__static_file__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__static_file__HttpResponseData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__static_file__LocationConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__AsyncRouter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__Channel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__Connection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__HandlerRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__HttpRequestData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__HttpRequestParser(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__HttpResponseData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__IoDriver(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__LocationConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__MiddlewarePipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__http_server__worker__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__buffer__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__event_loop__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__event_loop__IoEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__tcp__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__tcp__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__tcp__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__udp__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__udp__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__io__udp__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_handlers__DebugSession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_handlers__EvalResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_handlers__SessionBreakpoint(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_handlers__SessionDataBreakpoint(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_handlers__SessionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_log_tools__DebugLogEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_tools__DebugSession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_tools__EvalResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_tools__SessionBreakpoint(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_tools__SessionDataBreakpoint(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__debug_tools__SessionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_edit_tools__DiagDelta(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_edit_tools__DiagEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_edit_tools__DiagResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_read_tools__DiagEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_read_tools__DiagResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_vcs_tools__DiagEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__diag_vcs_tools__DiagResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__fileio_main__FileIOServer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__fileio_server__ProtectionEngine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__fileio_server__TempManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__handler_registry__PromptHandler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__handler_registry__ResourceHandler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__handler_registry__ToolHandler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__log_store__DebugLogEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__log_store__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__main__McpCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__main__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__outline_renderer__DiagEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__resource_utils__JsonArrayBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__mcp__resource_utils__JsonBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__Adam(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__ArrayDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__DataLoader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__LabeledBatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__LabeledDataLoader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__LabeledDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__LabeledSample(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__PureTensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__SGD(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__async_training__TrainingHistory(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__ml__data_pipeline__ArrayDataset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__thread_pool__ThreadHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__thread_pool__ThreadSafeQueue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__Adam(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__CrossEntropyLoss(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__MSELoss(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__RMSprop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__SGD(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__Sequential(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__mod__Stream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__torch_training__Conv2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__torch_training__Linear(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut__torch__torch_training__Tensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__baremetal__arm__startup__VectorTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__baremetal__arm__test_support__VectorTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__baremetal__riscv__startup__PlicContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__cli_integration__ExecutionConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__cli_integration__ExecutionResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__cli_integration__TestExecutor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__mod__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__mod__GdbMiAdapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__mod__LocalAdapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__semihost_capture__ExecutionConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__semihost_capture__ExecutionResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__semihost_capture__QemuConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__semihost_capture__QemuInstance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__semihost_capture__SemihostConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__execution__semihost_capture__TestExecutor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__boot_runner__ExitCodeResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__boot_runner__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__boot_runner__QemuConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__boot_runner__QemuInstance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__debug_boot_runner__BootTestConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__debug_boot_runner__BootTestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__debug_boot_runner__BootTestRunner(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__debug_boot_runner__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__debug_boot_runner__QemuConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__debug_boot_runner__QemuInstance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__mod__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__semihosting__QemuConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__ExecutionConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__ExecutionResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__GdbMiAdapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__QemuConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__QemuInstance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_async_mut_noalloc__qemu__test_session__TestExecutor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__allocator__AtomicBool(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__allocator__AtomicI64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__allocator__AtomicUsize(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__benchmark__mod__BenchmarkConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__benchmark__mod__BenchmarkStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__cli__mod__ParsedResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__cli__mod__SimpleParser(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__damage__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__gpu_command__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__gpu_command__GpuSurface(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__gpu_command__Rasterizer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__gpu_command__SurfacePool(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__rasterizer__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__rasterizer__GpuSurface(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__stacking__DisplayEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__compositor__tile__CompositeLayer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__client__DaemonConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__client__DaemonRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__client__DaemonResponse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__daemon__DaemonConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__daemon__DaemonRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__daemon__DaemonResponse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__daemon__DaemonState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__protocol__DaemonRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__daemon_sdk__protocol__DaemonResponse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__gdb_mi__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__InterpreterHookContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__StackFrame(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__local__Variable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__mod__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__mod__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__mod__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__openocd__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__openocd__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__openocd__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__openocd__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__openocd__OpenocdClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__openocd__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__remote__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__remote__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__remote__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__remote__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__remote__RemoteRiscV32Backend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__remote__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__stlink_tools__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__stlink_tools__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__stlink_tools__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__stlink_tools__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__stlink_tools__StLinkToolsClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__stlink_tools__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__t32_gdb__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__t32_gdb__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__t32_gdb__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__t32_gdb__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__t32_gdb__T32GdbBridgeClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__t32_gdb__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__AdapterCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__AdapterConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__LocationInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__adapter__trace32__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__dap_handlers__FaultConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__main__FaultConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__server__FaultConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__dap__transport__FaultConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__bug__DbIssue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__bug__SdnDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__bug__SdnRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__bug__SdnTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__bug__StringInterner(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__checker__SdnDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__checker__SdnRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__checker__SdnTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__feature__DbIssue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__query__SdnRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__query__SdnTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__async_wrapper__Database(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__async_wrapper__DbRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__async_wrapper__HostFuture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__async_wrapper__HostPromise(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__codec__DbRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__codec__TableSchema(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__connection__DbRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__connection__PreparedStatement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__connection__SqliteConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__connection__SqliteRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__connection__StatementCache(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__connection__Transaction(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__migration__Database(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__repository__Database(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__repository__DbRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__repository__SelectQuery(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__sql_gen__ColumnDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__sql_gen__TableSchema(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__statement__DbRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__statement__SqliteConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__statement__SqliteStatement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__stmt_cache__PreparedStatement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__stmt_cache__SqliteConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__stmt_cache__SqliteStatement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__transaction__DbRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__transaction__SqliteConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__sql__transaction__SqliteRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__task__DbIssue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__task__SdnDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__task__SdnRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__task__SdnTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test__DbIssue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__TestInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__core_helpers__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__TestInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__database__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__factory__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__queries__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__queries__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__queries__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__queries__TestInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__queries__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__runs__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__test_extended__tracking__TestDatabaseExtended(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__todo__DbIssue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__todo__SdnDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__todo__SdnRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__database__todo__SdnTable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__coordinator__SessionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ch32v307__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ch32v307__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ch32v307__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ghdl_rv32__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ghdl_rv32__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ghdl_rv32__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ghdl_rv32__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_ghdl_rv32__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_arm__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_arm__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_arm__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_arm__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_arm__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_rv32__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_rv32__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_rv32__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_rv32__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_qemu_rv32__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32h7__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32h7__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32h7__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32wb__CodeInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32wb__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32wb__ExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32wb__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32wb__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_stm32wb__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_trace32__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_trace32__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_trace32__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_trace32__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_trace32__T32GdbBridgeClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__adapter_trace32__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_registry__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_registry__LaneDescriptor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_status__CapabilityReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_status__LaneDescriptor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_status__LaneRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__manager__CodeInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__manager__CodeUploader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__manager__ExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__manager__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__manager__TargetMemoryAllocator(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__memory_alloc__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__result_collector__ExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__exec__uploader__CodeInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__feature__emulation__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__gdb_mi__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__openocd__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__openocd__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__openocd__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__openocd__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_gdb_bridge__DebugConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_gdb_bridge__FrameInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_gdb_bridge__GdbMiClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_gdb_bridge__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_gdb_bridge__VarInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_practice_runner__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_practice_runner__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_session_registry__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_session_registry__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_window_capture__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_window_capture__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__t32_window_capture__Trace32Parser(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_breakpoint__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_breakpoint__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_core__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_core__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_direct__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_direct__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_fdx__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_fdx__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_memory__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_memory__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_register__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_register__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_trace__T32Error(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__t32_ffi__t32_trace__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__test__qemu_runner__QemuConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__debug__remote__test__qemu_runner__QemuInstance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__AudioBus(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__AudioClip(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__AudioConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__Listener3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__SoundHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__audio_manager__Volume(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__types__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__types__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__audio__types__Volume(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera3d__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera3d__Mat4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera3d__Transform3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera__CameraData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__camera__ZoomLevel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__mesh__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__mesh__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__particle__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__particle__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__particle__RenderCommandBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__particle__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__particle__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__particle__ZIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry3d__Camera3DData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry3d__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry3d__LightData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry3d__Material3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry3d__MeshData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__AudioClipId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__Speed(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__registry__ZoomLevel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__AnimatedSpriteData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__Speed(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__SpriteData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__sprite__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__tilemap__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__tilemap__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__tilemap__TileCoord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__tilemap__TileIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__tilemap__TileMapData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__component__tilemap__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__AudioConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__AudioManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__ComponentRegistry3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__ForwardRenderer3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__InputManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__Material3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__MeshData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__Node3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__NodeStore3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__PhysicsConfig3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__PhysicsWorld3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__ResourceManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine3d__Window(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__AudioConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__AudioManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__ComponentRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__GpuRenderer2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__InputManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__Node2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__NodeStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__PhysicsConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__PhysicsWorld2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__ResourceManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__SoftwareRenderer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__VectorRenderer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__engine__Window(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__Clock(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__FrameTime(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop3d__Window(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__Clock(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__FrameTime(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__RenderCommandBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__game_loop__Window(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__core__time__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__default_bindings__ActionBinding(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__default_bindings__InputManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__default_bindings__KeyCode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__input_manager__ActionBinding(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__input_manager__KeyCode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__input_manager__MouseButtonId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__input_manager__MouseState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__input_manager__Signal(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__types__KeyCode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__input__types__MouseButtonId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__body2d__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__body3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__body3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__collision2d__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__collision3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__collision3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__contact2d__Contact2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__contact2d__InternalBody2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__contact2d__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__contact3d__Contact3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__contact3d__InternalBody3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__contact3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__debug_draw3d__PhysicsWorld3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__debug_draw3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__debug_draw3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__debug_draw__PhysicsWorld2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__debug_draw__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints3d__InternalBody3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints3d__Joint3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints3d__PhysicsWorld3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints__InternalBody2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints__Joint2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints__PhysicsWorld2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__joints__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query3d__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query3d__PhysicsWorld3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query3d__RaycastHit3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query__PhysicsWorld2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query__RaycastHit2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__query__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types3d__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types3d__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__types__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__ColliderShape3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__CollisionEvent3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__Contact3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__InternalBody3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__PhysicsBody3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__PhysicsCollider3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__PhysicsConfig3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__Signal(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__ColliderShape2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__CollisionEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__Contact2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__Distance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__InternalBody2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__PhysicsBody2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__PhysicsCollider2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__PhysicsConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__Seconds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__Signal(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__physics__world__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__batch__ZIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__command__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__command__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__command__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__command__Vertex2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__command__ZIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__DrawStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuFramebuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuGraphicsPipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuPipelineDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuRenderPass(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuRenderPassDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuShader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuSwapchain(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuSwapchainDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuTexture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuTextureDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuVertex2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuVertexAttribute(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__GpuVertexLayout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_bridge__SpriteBatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__DrawStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__GpuRenderState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__RenderCommandBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_pipeline__Vertex2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_texture_cache__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_texture_cache__GpuTexture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_texture_cache__GpuTextureDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_texture_cache__Texture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_texture_cache__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__gpu_texture_cache__TextureStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__material3d__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__material3d__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__material__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__material__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__DrawStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__RenderCommandBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__Texture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__TextureStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__pipeline__Vertex2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Camera3DData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__ComponentRegistry3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__DrawStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__LightData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Mat4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Material3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__MeshData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Node3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__NodeStore3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Transform3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Vec4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__renderer3d__Vertex3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__shader__GpuShader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__text__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__text__FontHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__text__GlyphBitmap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__text__GlyphMetrics(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__text__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__vector__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__vector__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__vector__RenderCommandBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__vector__Tolerance(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__render__vector__ZIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__resource__manager__AudioClip(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__resource__manager__Texture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__resource__manager__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__resource__manager__TextureStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__Generation(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__Mat4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__Quaternion(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__RawHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__Transform3D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node3d__Vec3(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__Generation(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__RawHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__Transform2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__node__ZIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__Angle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__Generation(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__Node2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__NodeStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__RawHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__serializer__ZIndex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__tree__Node2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__tree__NodeId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__tree__NodeStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__tree__Transform2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__scene__tree__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__atlas__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__atlas__Texture(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__sprite__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__sprite__Rect2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__sprite__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__sprite__Vec2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__texture__EngineColor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__texture__Generation(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__texture__RawHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__engine__sprite__texture__TextureId(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__circuit__FailSafeError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__CircuitBreaker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__CircuitBreakerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__CircuitBreakerRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__CircuitBreakerStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__ConsoleLogger(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__Counter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__Deadline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__FailSafeError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__Gauge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__MetricsRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__PanicHandler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__PanicInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__RateLimitConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__RateLimitStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__RateLimiter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__ResourceAlert(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__ResourceLimits(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__ResourceMonitor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__ResourceMonitorStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__ResourceUsage(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__TimeoutConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__TimeoutManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__TimeoutStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__TimeoutToken(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__mod__TokenBucket(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__panic__FailSafeError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__failsafe__ratelimit__FailSafeError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ffi__dynamic_versioned__DynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ffi__ffi_signature__VersionedDynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ffi__llvm__DynLib(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gc__ArenaAllocator(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gc__AtomicBool(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gc__RuntimeValue(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gc__SlabAllocator(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gpu__context__GpuArray(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gpu__context__Stream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gpu__device__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gpu__mod__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__gpu__mod__GpuArray(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncBufferedReader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncBufferedWriter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncFile(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncFileHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncTcpListener(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncTcpStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AsyncUdpSocket(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AudioEngine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AudioPlayback(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__AudioSource(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Bounds(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__BufferedReader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__BufferedWriter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ButtonData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Collider(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Contact(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ContactList(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__CoverageStats(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Event(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__EventBatch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__File(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__FileHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__FileMetadata(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__FillTessellation(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__FontHandle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__FtpConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__GamepadContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__GamepadEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__GlyphBitmap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__GlyphMetrics(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__HttpClient(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__HttpResponse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__HttpServer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ImageData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__IndexBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__IoEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Joint(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__KeyboardEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Monitor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__MouseButtonEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__MousePosition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__MouseWheel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Path(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__PathBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__PhysicsMaterial(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__PhysicsWorld(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Point2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Position(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__PowerInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ProfileTimer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__RayCastResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Rect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Regex(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__RigidBody(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Rotation2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__RumbleEffect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SffiResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SftpFileInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SftpSession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Size(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SqliteConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SqliteRow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SqliteStatement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SshChannel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SshExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__SshSession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Stderr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Stdin(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Stdout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__StickState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__StrokeOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__StrokeTessellation(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TarArchive(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TcpListener(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TcpStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ThreadPool(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsCertificate(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsClientConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsClientConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsConnectionInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsServer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TlsServerConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Transform2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Transform2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__TriggerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__UdpSocket(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Vector2(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Vector2D(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__VertexBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__VhdlToolResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Waker(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__WebSocket(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__Window(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__WindowConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__ZipArchive(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io___BreakpointEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__buffer__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__crypto_ffi__SffiResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__file__FileMetadata(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__file__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__pipe__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__process_limit_enforcer__ResourceProfile(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__tcp__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__io__udp__IoError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_backend__JitBinary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_backend__JitExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_backend__JitLayoutPlan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_backend__JitMemoryLimits(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_layout__JitBinary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_layout__JitLayoutPlan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_layout__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__DisconnectedBackendOps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__JitBackendOps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__JitBinary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__JitExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__JitLayoutPlan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__JitMemoryLimits(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__jit__jit_runner__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__lsp__main_wasi__ParserAdapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__lsp__transport__FaultConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp__jj__main__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp__jj__warning__JjResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__McpCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__McpError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__PromptDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__ResourceDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__ServerInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__ToolDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__core__mod__ValidationLimits(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__server__builder__McpCapabilities(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__server__builder__PromptDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__server__builder__ResourceDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__server__builder__ServerInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__server__builder__ToolDef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mcp_sdk__server__router__ServerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mem_tracker__mod__MemDiff(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mem_tracker__mod__MemLeakEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mem_tracker__mod__MemLeakReport(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mem_tracker__mod__MemPhaseSnapshot(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mem_tracker__mod__MemSnapshot(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__mem_tracker__mod__MemTagGroup(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__message_transfer__BinaryRef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__message_transfer__SharedHeap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__message_transfer__SharedHeapConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__Backtrace(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__Context(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__ErrorChain(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__StackFrame(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__ffi__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__http__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__tcp__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__telnet__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__telnet__TcpStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__net__udp__SimpleError(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__backend_dpkg__InstallerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__backend_dpkg__InstallerResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__backend_fpm__InstallerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__backend_fpm__InstallerResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__backend_nsis__InstallerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__backend_nsis__InstallerResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__staging__InstallerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__installer__tool_detect__ToolAvailability(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__manifest__Dependency(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__manifest__Manifest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__manifest__PackageInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__package__manifest__Version(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__convert__PlatformConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__mod__PlatformConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__mod__WireReader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__mod__WireWriter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__newline__PlatformConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__text_io__PlatformConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__platform__wire__PlatformConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ptr__mod__Handle(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ptr__mod__PtrState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ptr__mod__UniquePtr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ptr__mod__WeakRef(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__resource_tracker__ProcessMetrics(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__runtime_value__AtomicI64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__sanitizer__mod__SanEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__spec__decorators__SkipCondition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__spec__mod__FeatureMetadata(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__spec__mod__SkipCondition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__aop__AopConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__context_manager__TimerContext(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__core__decorators__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__decorators__CachedFunction(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__decorators__DeprecatedFunction(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__decorators__LoggedFunction(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__dl__config_loader__DLConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__infra__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__set__Map(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__set__MapEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__Gpu(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__GpuBuffer(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__GpuDeviceEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__GpuEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__GpuKFunc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__GpuMemoryPool(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__GpuStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__testing__gpu_helpers__KernelLaunch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__tooling__easy_fix__rules__EasyFix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__src__tooling__easy_fix__rules__Replacement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__connection__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__connection__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__connection__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__connection__TerminalConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__connection__TerminalExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__credential__config_parser__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__credential__config_parser__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__controller__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__controller__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__host_power__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__host_power__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__host_power__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__relay_power__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__relay_power__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__t32_power__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__t32_power__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__power__t32_power__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__relay_terminal__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__relay_terminal__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__relay_terminal__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__relay_terminal__TerminalConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__relay_terminal__TerminalExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__SftpFileInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__SftpSession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__SshChannel(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__SshExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__SshSession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__TerminalConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__ssh_terminal__TerminalExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__ProcessResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__TerminalConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__TerminalExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__t32_swd_terminal__Trace32Client(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__telnet_terminal__PowerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__telnet_terminal__PowerState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__telnet_terminal__TelnetConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__telnet_terminal__TerminalConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__telnet_terminal__TerminalConnection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__terminal__telnet_terminal__TerminalExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__IndividualTestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__TestDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__TestFailure(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__TimingConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doc_generator__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doctest_runner__BlockResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doctest_runner__SdoctestBlock(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doctest_runner__SdoctestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doctest_runner__SdoctestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__doctest_runner__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__execution_strategy__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__CoverageData(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__Doctest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__ExecutionStrategy(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__FileCoverage(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__IndividualTestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__RustTestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__SequentialContainerConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__SkipFeatureInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestFailure(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TimingConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__main__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__sequential_container__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__sequential_container__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__IndividualTestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__ParsedStableDb(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__ParsedVolatileDb(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__StringInterner(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__TestFailure(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__TimingConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_core__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_io__FileLock(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__StringBuilder(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__StringInterner(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_parser__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__ChangeEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__CounterRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__FileRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__RunRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__StringInterner(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__SuiteRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__TestRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__TimingRun(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_db_serializer__TimingSummary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite__JitBinary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__Ch32V307Adapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__ExecResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__GhdlRv32Adapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__JitBinary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__MemoryMap(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__QemuArmAdapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__QemuRv32Adapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__RemoteExecutionManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__Stm32H7Adapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__Stm32WbAdapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_composite_jit_generic__Trace32Adapter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_parsing__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_executor_parsing__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_manifest_scanner__SdoctestManifestEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_manifest_scanner__TestManifest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_manifest_scanner__TestManifestEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_args__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_config__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_db__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_execute__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_execute__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_files__SkipFeatureInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_files__TestManifest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_files__TestManifestEntry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_files__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_fork__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_fork__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_helpers__IndividualTestResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_helpers__TestDatabase(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_helpers__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_main__TestConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_main__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_main__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_main__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_modes__TestConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_modes__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_modes__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_output__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_output__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_single__TestFileResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_single__TestOptions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__test_runner__test_runner_single__TestRunResult(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__Adam(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__CrossEntropyLoss(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__MSELoss(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__NogradGuard(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__RMSprop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__SGD(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__Sequential(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__mod__Stream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__torch_training__Conv2d(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__torch_training__Linear(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__torch__torch_training__Tensor(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__layout__Rect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widget__Style(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widget__StyledLine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widget__StyledSegment(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__box_widget__Rect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__box_widget__Style(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__box_widget__StyledLine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__box_widget__StyledSegment(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__input__Rect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__input__Style(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__input__StyledLine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__input__StyledSegment(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__list__Rect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__list__Style(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__list__StyledLine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__text__Rect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__text__Style(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__text__StyledLine(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__tui__widgets__text__StyledSegment(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ui_test__client__ElementInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ui_test__client__UIStateInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ui_test__http__TcpStream(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ui_test__parse__ElementInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__ui_test__parse__UIStateInfo(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__unsafe__mod__MaybeUninit(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__CommandRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__EngineDomBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__EventBus(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__InputBridge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__Logger(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__Payload(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__PluginRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__RenderBridge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__WebUIWindow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__app__WindowManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__bridge__DisplayItem(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__bridge__DisplayList(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__bridge__Logger(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__bridge__RenderPipeline(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__Document(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__DomNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__ElementOps(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__Logger(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__NodeStore(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__ScriptToDomBridge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__WebElement(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__dom_backend__WebEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__AppConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__CommandRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__EngineDomBackend(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__EventBus(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__FsPlugin(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__InputBridge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__Payload(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__PluginRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__RenderBridge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__WebUIApp(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__WebUIWindow(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__mod__WindowManager(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__plugin__CommandRegistry(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__plugin__EventBus(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__window__EventLoop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__window__Logger(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__window__RenderBridge(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue nogc_sync_mut__web_ui__window__Window(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue platform__PlatformConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue platform__WireReader(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue platform__WireWriter(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue process_monitor__ProcessMetrics(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue resource_tracker__ResourceUsageRecord(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__display__display_service__DamageRect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__display__display_service__DisplayMode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__display__display_service__SurfaceDesc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__cli_gui_bridge__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__cli_gui_bridge__UIState(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__cli_gui_bridge__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__cli_gui_bridge__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__mcp_os_server__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__mcp_os_server__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__mcp_os_server__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__tool_registry__ToolHandler(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__widget_eval__UISession(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__widget_eval__UITree(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__llm__widget_eval__WidgetNode(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmCloseRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmCreateRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmCreateResponse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmFocusEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmInputEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmMoveRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue services__wm__wm_service__WmResizeRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmCloseRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmCreateRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmCreateResponse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmFocusEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmInputEvent(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmMoveRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

RuntimeValue userlib__window__WmResizeRequest(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(128);
}

/* ===================================================================
 * 5. _dot_ method stubs - 66 total
 *
 * Symbols like ADMIN_QUEUE_DEPTH_dot_to_u64 are method calls.
 * - *_dot_to_u64, *_dot_to_int, *_dot_to_text: converter stubs
 * - *_dot_eq, *_dot_is_*: predicate stubs (return false)
 * - *_dot_from_*, *_dot_new: factory stubs
 * - Enum variant constructors (_dot_Variant): tagged integer
 * - Others: self passthrough
 * =================================================================== */

RuntimeValue ADMIN_QUEUE_DEPTH_dot_to_u64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue ArmCortexMTarget_dot_cortex_m4(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue ArmCortexMTarget_dot_cortex_m7(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue Diagnostic_dot_help_msg(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue Diagnostic_dot_note(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue ElementOps_dot_getAttribute(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue ElementOps_dot_removeAttribute(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue ElementOps_dot_setAttribute(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue FsNodeKind_dot_from_u8(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* factory: pass through first arg */
}

RuntimeValue IO_QUEUE_DEPTH_dot_to_u64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue SSH_MIN_PADDING_dot_to_u64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue String_dot_from_byte(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* factory: pass through first arg */
}

RuntimeValue TCP_MSS_dot_to_u64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue ThreadHandle_dot_from_raw(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* factory: pass through first arg */
}

RuntimeValue Trace32Parser_dot_tokenize_whitespace(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue VIRTIO_BLK_CFG_BLK_SIZE_dot_to_u64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue VIRTIO_BLK_CFG_CAPACITY_dot_to_u64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue VersionConstraint_dot_GreaterEq(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue VersionConstraint_dot_LessThan(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(1); /* enum variant tag */
}

RuntimeValue VersionConstraint_dot_Range(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(2); /* enum variant tag */
}

RuntimeValue Waker_dot_from_fn(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* factory: pass through first arg */
}

RuntimeValue async__os_poll__OsPoll_dot_Ready(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue kernel__arch__arch_context__ArchContext_dot_Riscv32(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue kernel__arch__arch_context__ArchContext_dot_Riscv64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(1); /* enum variant tag */
}

RuntimeValue kernel__arch__arch_context__ArchContext_dot_X86_64(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(2); /* enum variant tag */
}

RuntimeValue kernel__arch__arch_context__ArchContext_dot_arch(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* method: self passthrough */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_FileCreate(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_FileRead(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(1); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_FileWrite(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(2); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_IpcConnect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(3); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_IpcListen(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(4); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_Mmio(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(5); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_NetConnect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(6); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_NetListen(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(7); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_PortIO(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(8); /* enum variant tag */
}

RuntimeValue kernel__types__capability_types__CapabilityKind_dot_ProcessSignal(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(9); /* enum variant tag */
}

RuntimeValue kernel__types__task_types__BlockReason_dot_IpcRecv(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue kernel__types__task_types__BlockReason_dot_Notification(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(1); /* enum variant tag */
}

RuntimeValue kernel__types__task_types__TaskState_dot_Blocked(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__debug__remote__connection_matrix__DebugTarget_dot_eq(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__debug__remote__connection_matrix__DebuggerKind_dot_eq(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__capability_report__CapabilityStatus_dot_is_acceptable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__capability_report__CapabilityStatus_dot_is_runnable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__capability_report__CapabilityStatus_dot_to_text(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* to_text: would need string alloc */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_descriptor__AdapterKind_dot_to_text(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* to_text: would need string alloc */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_descriptor__LaneStatus_dot_is_authoritative(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_descriptor__LaneStatus_dot_to_text(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* to_text: would need string alloc */
}

RuntimeValue nogc_sync_mut__debug__remote__exec__lane_descriptor__ProofClass_dot_to_text(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* to_text: would need string alloc */
}

RuntimeValue nogc_sync_mut__debug__remote__protocol__gdb_mi_parser__GdbMiRecord_dot_is_stopped(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__failsafe__core__ErrorCategory_dot_is_recoverable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* predicate: false */
}

RuntimeValue nogc_sync_mut__lsp__protocol__DiagnosticSeverity_dot_to_int(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* converter: pass through (already int-tagged) */
}

RuntimeValue nogc_sync_mut__package__dist__Platform_dot_all(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* collection: would need array alloc */
}

RuntimeValue nogc_sync_mut__package__installer__types__InstallerPlatform_dot_all(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* collection: would need array alloc */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_Caret(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_Exact(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(1); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_Greater(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(2); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_GreaterEqual(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(3); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_Less(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(4); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_LessEqual(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(5); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__package__types__VersionConstraint_dot_Tilde(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(6); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__test_runner__sdoctest__types__SdoctestModifier_dot_Env(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__test_runner__sdoctest__types__SdoctestModifier_dot_Init(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(1); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__test_runner__sdoctest__types__SdoctestModifier_dot_RunConfig(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(2); /* enum variant tag */
}

RuntimeValue nogc_sync_mut__test_runner__sdoctest__types__SdoctestModifier_dot_Tag(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(3); /* enum variant tag */
}

RuntimeValue text_dot_from_any(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* factory: pass through first arg */
}

RuntimeValue text_dot_from_utf8(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return a; /* factory: pass through first arg */
}

/* ===================================================================
 * 6. Collection runtime stubs - 48 total
 *
 * BTreeMap, BTreeSet, HashMap, HashSet runtime functions.
 * On baremetal, these return empty/nil results.
 * =================================================================== */

RuntimeValue __rt_btreemap_clear(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_btreemap_contains_key(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_btreemap_entries(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreemap_first_key(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* not found */
}

RuntimeValue __rt_btreemap_get(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* not found */
}

RuntimeValue __rt_btreemap_insert(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_btreemap_keys(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreemap_last_key(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* not found */
}

RuntimeValue __rt_btreemap_len(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* empty */
}

RuntimeValue __rt_btreemap_new(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(64); /* empty collection */
}

RuntimeValue __rt_btreemap_remove(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_btreemap_values(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreeset_clear(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_btreeset_contains(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_btreeset_difference(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreeset_first(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* not found */
}

RuntimeValue __rt_btreeset_insert(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_btreeset_intersection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreeset_is_subset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_btreeset_is_superset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_btreeset_last(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* not found */
}

RuntimeValue __rt_btreeset_len(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* empty */
}

RuntimeValue __rt_btreeset_new(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(64); /* empty collection */
}

RuntimeValue __rt_btreeset_remove(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_btreeset_symmetric_difference(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreeset_to_array(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_btreeset_union(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashmap_contains_key(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_hashmap_entries(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashmap_get(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* not found */
}

RuntimeValue __rt_hashmap_insert(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_hashmap_keys(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashmap_len(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* empty */
}

RuntimeValue __rt_hashmap_new(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(64); /* empty collection */
}

RuntimeValue __rt_hashmap_remove(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_hashmap_values(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashset_contains(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_hashset_difference(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashset_drop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_hashset_insert(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_hashset_intersection(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashset_is_subset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_hashset_is_superset(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* false */
}

RuntimeValue __rt_hashset_len(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return ENCODE_INT(0); /* empty */
}

RuntimeValue __rt_hashset_new(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return alloc_struct(64); /* empty collection */
}

RuntimeValue __rt_hashset_remove(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* no-op */
}

RuntimeValue __rt_hashset_symmetric_difference(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

RuntimeValue __rt_hashset_to_array(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE; /* empty collection result */
}

/* __traits marker */
RuntimeValue __traits(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* ===================================================================
 * 7. Host-only function stubs - 114 total
 *
 * Functions that require host OS services (networking, filesystem,
 * async runtime, logging, etc.). Return NIL or 0 on baremetal.
 * =================================================================== */

/* --- async runtime --- */

RuntimeValue async_join(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue async_read_file(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue async_select(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue async_sleep(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue async_write_file(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue async_yield(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- backtrace capture --- */

RuntimeValue capture_backtrace(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue capture_backtraces(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- debug logging --- */

RuntimeValue debug_log_clear(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue debug_log_disable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue debug_log_enable(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue debug_log_get_entries(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue debug_log_get_entries_since(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue debug_log_get_status(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- dependency injection --- */

RuntimeValue di_register(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue di_resolve(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue di_setup_from_config(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- directory operations --- */

RuntimeValue dir_read(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- LSP engine --- */

RuntimeValue engine_completions(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue engine_document_symbols(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue engine_find_definition(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue engine_find_references(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue engine_hover(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue engine_signature_help(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue engine_type_at(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- filesystem --- */

RuntimeValue fs_read_text(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue fs_write_text(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- async futures --- */

RuntimeValue future_alloc_pending(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue future_alloc_ready(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue future_free(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue future_map(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue future_poll(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue future_poll_any(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue future_then(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- logging --- */

RuntimeValue log_clear_scope_levels(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_debug_with_prefix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_emit(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_error_with_prefix(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_get_global_level(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_get_scope_level(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_is_enabled(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_set_global_level(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue log_set_scope_level(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- memory management --- */

RuntimeValue memory_init(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- MMIO (host) --- */

RuntimeValue mmio_read8(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- physical memory (host) --- */

RuntimeValue pmm_alloc_page(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- async promises --- */

RuntimeValue promise_alloc(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue promise_complete(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue promise_free(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue promise_get_future(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue promise_is_completed(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- I/O read --- */

RuntimeValue read_header(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- stdin readline --- */

RuntimeValue readln(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- test scheduling --- */

RuntimeValue schedule_session_file_paths(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue schedule_session_tests(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue schedule_summary(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- search --- */

RuntimeValue search_recursive(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- self-extraction --- */

RuntimeValue self_extract_create(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue self_extract_get_ratio(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue self_extract_is_compressed(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- contract checks --- */

RuntimeValue simple_contract_check(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue simple_contract_check_msg(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- stdin --- */

RuntimeValue stdin_read_char(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- TCP networking --- */

RuntimeValue tcp_listener_accept(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_listener_accept_timeout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_listener_bind(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_listener_close(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_listener_local_addr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_listener_set_nonblocking(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_close(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_connect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_connect_timeout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_flush(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_local_addr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_peer_addr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_read(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_read_exact(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_read_line(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_set_nodelay(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_set_nonblocking(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_set_read_timeout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_set_write_timeout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_write(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue tcp_stream_write_all(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- test infrastructure --- */

RuntimeValue test_daemon_config_default(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue test_daemon_ensure_running(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue test_session_meta_default(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue test_submit_and_wait(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue test_user_service(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- UDP networking --- */

RuntimeValue udp_socket_bind(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_close(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_connect(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_join_multicast(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_leave_multicast(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_local_addr(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_recv(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_recv_from(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_send(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_send_to(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_set_broadcast(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_set_multicast_loop(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_set_nonblocking(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue udp_socket_set_read_timeout(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- UPX compression --- */

RuntimeValue upx_compress_file(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue upx_decompress_file(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue upx_ensure_available(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue upx_get_path(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue upx_get_ratio(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue upx_is_compressed(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- URL parsing --- */

RuntimeValue url_parse(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- directory walking --- */

RuntimeValue walk_dir(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

/* --- I/O write --- */

RuntimeValue write_header(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue write_header_field(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

RuntimeValue write_message(RuntimeValue a, RuntimeValue b, RuntimeValue c, RuntimeValue d,
        RuntimeValue e, RuntimeValue f, RuntimeValue g, RuntimeValue h) {
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    return NIL_VALUE;
}

