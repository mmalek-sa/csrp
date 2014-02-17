#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#include "srp.h"


#define NITER          1
#define TEST_HASH      SRP_SHA1
#define TEST_NG        SRP_NG_1024

unsigned long long get_usec()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (((unsigned long long)t.tv_sec) * 1000000) + t.tv_usec;
}

const char * test_n_hex = "EEAF0AB9ADB38DD69C33F80AFA8FC5E86072618775FF3C0B9EA2314C9C256576D674DF7496"
   "EA81D3383B4813D692C6E0E0D5D8E250B98BE48E495C1D6089DAD15DC7D7B46154D6B6CE8E"
   "F4AD69B15D4982559B297BCF1885C529F566660E57EC68EDBC3C05726CC02FD4CBF4976EAA"
   "9AFD5138FE8376435B9FC61D2FC0EB06E3";
const char * test_g_hex = "2";


void unsigned_to_hex(char* name, const unsigned char* input) {
    int i;
    printf("%s : ", name);
    for (i = 0; i < sizeof(input)-1; i++)
    {
        printf("%02X", input[i]);
    }
    printf("\n");
}

void to_hex(char* name, const char* input) {
    
    if (input == NULL)
        return;
    
    int i;
    printf("%s : ", name);
    for (i = 0; i < sizeof(input); i++)
    {
        printf("%02X", input[i]);
    }
    printf("\n");
}

int main( int argc, char * argv[] )
{
    struct SRPVerifier * ver;
    struct SRPUser     * usr;
    
    const unsigned char * bytes_s = 0;
    const unsigned char * bytes_v = 0;
    const unsigned char * bytes_A = 0;
    const unsigned char * bytes_B = 0;
    
    const unsigned char * bytes_M    = 0;
    const unsigned char * bytes_HAMK = 0;
    
    int len_s   = 0;
    int len_v   = 0;
    int len_A   = 0;
    int len_B   = 0;
    int len_M   = 0;
    int i;
    
    unsigned long long start;
    unsigned long long duration;
    
    const char * username = "mani_bono@yahoo.com";
    const char * password = "eybaba13";
    
    const char * auth_username = 0;
    const char * n_hex         = 0;
    const char * g_hex         = 0;
    
    SRP_HashAlgorithm alg     = TEST_HASH;
    SRP_NGType        ng_type = SRP_NG_8192; //TEST_NG;
    
    if (ng_type == SRP_NG_CUSTOM)
    {
        n_hex = test_n_hex;
        g_hex = test_g_hex;
    }

    
    srp_create_salted_verification_key( alg, ng_type, username, 
                (const unsigned char *)password, 
                strlen(password), 
                &bytes_s, &len_s, &bytes_v, &len_v, n_hex, g_hex );
    
    unsigned_to_hex("v", bytes_v);
    unsigned_to_hex("s", bytes_s);
    to_hex("n_hex", n_hex);
    to_hex("g_hex", g_hex);
    
    start = get_usec();
    
    for( i = 0; i < NITER; i++ )
    {
        usr =  srp_user_new( alg, ng_type, username, 
                             (const unsigned char *)password, 
                             strlen(password), n_hex, g_hex );
        

        srp_user_start_authentication( usr, &auth_username, &bytes_A, &len_A );

        /* User -> Host: (username, bytes_A) */
        ver =  srp_verifier_new( alg, ng_type, username, bytes_s, len_s, bytes_v, len_v, 
                                 bytes_A, len_A, & bytes_B, &len_B, n_hex, g_hex );
        
        printf("alg: %u \n", alg);
        printf("ng_type: %u \n", ng_type);
        printf("Bytes A: %u", bytes_A);
        unsigned_to_hex("A", bytes_A);
        unsigned_to_hex("B", bytes_B);
        
        if ( !bytes_B )
        {
            printf("Verifier SRP-6a safety check violated!\n");
            goto cleanup;
        }
        
        /* Host -> User: (bytes_s, bytes_B) */
        srp_user_process_challenge( usr, bytes_s, len_s, bytes_B, len_B, &bytes_M, &len_M );
        
        if ( !bytes_M )
        {
            printf("User SRP-6a safety check violation!\n");
            goto cleanup;
        }
        
        /* User -> Host: (bytes_M) */
        srp_verifier_verify_session( ver, bytes_M, &bytes_HAMK );
        
        if ( !bytes_HAMK )
        {
            printf("User authentication failed!\n");
            goto cleanup;
        }
        else {
            printf("Server authentication succeded!!\n");
        }
        
        /* Host -> User: (HAMK) */
        srp_user_verify_session( usr, bytes_HAMK );
        
        if ( !srp_user_is_authenticated(usr) )
        {
            printf("Server authentication failed!\n");
        }
        else {
            printf("Server authentication succeded!!\n");
        }
        
cleanup:
        srp_verifier_delete( ver );
        srp_user_delete( usr );
    }
    
    duration = get_usec() - start;
    
    printf("Usec per call: %d\n", (int)(duration / NITER));
    
    
    free( (char *)bytes_s );
    free( (char *)bytes_v );
        
    return 0;
}
