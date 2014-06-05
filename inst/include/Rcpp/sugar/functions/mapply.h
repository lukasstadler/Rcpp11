#ifndef Rcpp__sugar__mapply_h
#define Rcpp__sugar__mapply_h

namespace Rcpp{
    namespace sugar{

        template <typename input_type, bool>
        struct mapply_iterator_dispatch ;
        
        template <typename input_type>
        struct mapply_iterator_dispatch<input_type, true> {
            typedef constant_iterator<typename std::decay<input_type>::type > type ;
        } ;
        
        template <typename input_type>
        struct mapply_iterator_dispatch<input_type, false> {
            typedef typename input_type::expr_type::const_iterator type ;
        } ;
        
        template <typename input_type>
        struct mapply_iterator {
            typedef typename mapply_iterator_dispatch< 
                typename std::decay<input_type>::type, 
                Rcpp::traits::is_primitive<input_type>::value 
            >::type type ;
        } ;
        
        template <
            typename Function,
            typename... Args
        >
        class Mapply :
            public SugarVectorExpression<
                typename std::result_of<Function(typename traits::mapply_scalar_type<Args>::type ...)>::type, 
                Mapply<Function,Args...>
            >, 
            public custom_sugar_vector_expression
        {
        public:
            const static int N = sizeof...(Args);
            typedef typename Rcpp::traits::index_sequence<Args...>::type Sequence ;
            typedef std::tuple< typename std::conditional<traits::is_primitive<Args>::value, typename std::decay<Args>::type, Args>::type ...> Tuple ;
            typedef std::tuple< typename traits::mapply_scalar_type<Args>::type ... > ETuple ;
            typedef typename std::result_of<Function(typename traits::mapply_scalar_type<Args>::type ...)>::type real_value_type ;
            typedef typename std::conditional< std::is_same<bool,real_value_type>::value, Rboolean, real_value_type>::type value_type ;
            typedef std::tuple< typename mapply_iterator<Args>::type ... > IteratorsTuple ;
            
        private:
            Tuple data ;
            Function fun ;
            R_xlen_t n ;

        public:
                 
            class MapplyIterator {
            public:
                typedef std::bidirectional_iterator_tag iterator_category ;
                typedef int difference_type ;
                typedef typename Mapply::value_type value_type ;
                typedef value_type reference ;
                typedef value_type* pointer ;
                
                MapplyIterator( const Tuple& data, const Function& fun_, int pos = 0 ) : 
                    iterators( get_iterators(data, pos, Sequence() ) ), fun(fun_), index(pos)
                {}
                
                inline MapplyIterator& operator++(){
                    increment_all( Sequence() ) ;
                    ++index ;
                    return *this ;
                }
                
                inline MapplyIterator& operator--(){
                    decrement_all( Sequence() ) ;
                    --index ;
                    return *this ;
                }
                
                inline MapplyIterator& operator+=(int n){
                    increment_all( n, Sequence() ) ;
                    index += n ;
                    return *this ;
                }
                
                inline MapplyIterator& operator-=(int n){
                    decrement_all( n, Sequence() ) ;
                    index -= n ;
                    return *this ;
                }
                
                MapplyIterator operator+( int n){
                    MapplyIterator copy(*this) ;
                    copy += n;
                    return copy ;
                }
                
                MapplyIterator operator-( int n){
                    MapplyIterator copy(*this) ;
                    copy -= n;
                    return copy ;
                }
                
                inline value_type operator*() {
                    return apply( Sequence() ) ;
                }
                
                inline bool operator==( const MapplyIterator& other ){ return index == other.index; }
                inline bool operator!=( const MapplyIterator& other ){ return index != other.index; }
                
            private:
                IteratorsTuple iterators ;
                const Function& fun ;
                int index ;
                
                template <typename... Pack>
                void nothing( Pack... pack ){}
                
                template <int... S>
                void increment_all(Rcpp::traits::sequence<S...>) {
                    nothing( increment<S>()... ) ;    
                }
                
                template <int... S>
                void increment_all(int n, Rcpp::traits::sequence<S...>) {
                    nothing( increment<S>(n)... ) ;    
                }
                
                template <int... S>
                void decrement_all(Rcpp::traits::sequence<S...>) {
                    nothing( decrement<S>()... ) ;    
                }
                
                template <int... S>
                void decrement_all(int n, Rcpp::traits::sequence<S...>) {
                    nothing( decrement<S>(n)... ) ;    
                }
                
                template <int S>
                int increment(){
                    ++std::get<S>(iterators) ;
                    return 0  ;
                }
                
                template <int S>
                int increment(int n){
                    std::get<S>(iterators) += n;
                    return 0  ;
                }
                
                template <int S>
                int decrement(){
                    --std::get<S>(iterators) ;
                    return 0  ;
                }
                
                template <int S>
                int decrement(int n){
                    std::get<S>(iterators) -= n;
                    return 0  ;
                }
                
                template <int... S>
                value_type apply(Rcpp::traits::sequence<S...>) {
                    ETuple values( *std::get<S>(iterators) ... ) ;
                    auto tests = { (std::get<S>(values) == NA) ... } ;
                    if( std::any_of(tests.begin(), tests.end(), [](bool b){ return b ; } ) ) 
                        return NA ;
                    return internal::caster<real_value_type,value_type>(fun( std::get<S>(values)... )) ;
                } 
                 
                template <int... S>
                IteratorsTuple get_iterators(const Tuple& data, int pos, Rcpp::traits::sequence<S...>){
                    return std::make_tuple( get_iterator<S>( data, pos, 
                        typename Rcpp::traits::is_primitive< typename std::tuple_element<S,Tuple>::type >::type()
                    ) ... ) ;    
                }
                   
                template <int INDEX>
                inline typename std::tuple_element<INDEX,IteratorsTuple>::type get_iterator( const Tuple& data, int pos, std::true_type ) const {
                    return typename std::tuple_element<INDEX,IteratorsTuple>::type( std::get<INDEX>(data) ) ; 
                }
                
                template <int INDEX>
                inline typename std::tuple_element<INDEX,IteratorsTuple>::type get_iterator( const Tuple& data, int pos, std::false_type ) const {
                    return sugar_begin( std::get<INDEX>(data) ) + pos ;
                }

            } ;
            
            typedef MapplyIterator const_iterator ;
            
            Mapply( Function fun_, Args&&... args ) :
                data( args... ),
                fun(fun_),
                n(get_size())
            {
                RCPP_DEBUG( "Mapply = %s\n", DEMANGLE(Mapply) )
                RCPP_DEBUG( "Tuple  = %s\n", DEMANGLE(Tuple) )
                RCPP_DEBUG( "ETuple = %s\n", DEMANGLE(ETuple) ) 
                RCPP_DEBUG( "ITuple = %s\n", DEMANGLE(IteratorsTuple) )
            }

            inline R_xlen_t size() const {
                return n ;
            }
            inline const_iterator begin() const { return const_iterator( data, fun, 0 ) ; }
            inline const_iterator end() const { return const_iterator( data, fun, size() ) ; }
               
            template <typename Target>
            void apply( Target& target ) const {
                typedef typename traits::r_vector_element_converter< Target::r_type::value >::type converter ;
                std::transform( begin(), end(), target.begin(), [](value_type x){
                        return converter::get(x) ;
                });  
            }
            
        private: 
            inline int get_size() const {
                return get_size_impl( Sequence() ) ;    
            }
            
            template <int... S>
            R_xlen_t get_size_impl(Rcpp::traits::sequence<S...>) const {
                auto sizes = { 
                    get_ith_size<S>(
                        typename Rcpp::traits::is_primitive< typename std::tuple_element<S,Tuple>::type >::type()
                    ) ... 
                } ;
                return *std::max_element( std::begin(sizes), std::end(sizes) ) ;
            }
            
            template <int INDEX>
            constexpr R_xlen_t get_ith_size( std::true_type ) const {
                return 1 ;     
            }
            
            template <int INDEX>
            R_xlen_t get_ith_size( std::false_type ) const {
                return std::get<INDEX>(data).size() ;     
            }
            
        } ;

    } // sugar

    template <typename Function,typename... Args>
    typename sugar::Mapply<Function,Args...>
    mapply( Function fun, Args&&... args ){
        return sugar::Mapply<Function,Args...>( fun, std::forward<Args>(args) ... ) ;
    }

    template <typename Function,typename... Args>
    typename sugar::Mapply<Function,Args...>
    Map( Function fun, Args&&... args ){
        return sugar::Mapply<Function,Args...>( fun, std::forward<Args>(args) ... ) ;
    }

} // Rcpp

#endif
