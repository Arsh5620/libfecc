#include "forwardecc.h"
#include "rs.h"
#include <time.h>

int
cmp_blocks_equal (ff_t *block1, ff_t *block2, int size)
{
    return (memcmp (block1, block2, size) == 0);
}

void
init_message (poly_s poly)
{
    for (int i = 0; i < poly.size; ++i)
    {
        poly.memory[i] = i;
    }
}

void
corrupt_message1 (poly_s poly, int error_count)
{
    int error_distance = (poly.size / error_count);
    for (long i = 0; i < error_count; i++)
    {
        poly.memory[(i * error_distance + 11) % poly.size] = 0;
    }
}

void
corrupt_message2 (poly_s poly, int start_at, int error_count)
{
    int at_end = ((start_at + error_count) % poly.size) - start_at;
    memset (poly.memory + start_at, 0, at_end);

    if (error_count - at_end > 0)
    {
        memset (poly.memory, 0, error_count - at_end);
    }
}

int
main (int argc, char **argv)
{
    if (argc < 2){
        printf("usage <program> <127-253> [iteration count, default: 2Million]");
        exit(1);
    }

    int ecc = strtol(argv[1], NULL, 10);
    if (ecc <127 || ecc > 253){
        printf("Value not in the range 127-253");
        exit(1);
    }
    
    size_t iteration = 2000000;
    if (argc == 3){
        iteration = strtol(argv[2], NULL, 10);
        if (iteration < 20000){
            printf("Although lower iteration count will finish the test quicker,"
                " but depending upon the timer resolution it will probably not give an accurate result\n\n");
        }
    }
    
    for (size_t ecase = 0; ecase < 2; ecase++)
    {
        short n = 255, k = ecc;
        rs_encode_s encode = rs_init_encoder (n, k, 285);
        rs_decode_s decode = rs_init_decoder (n, k, 285);

        init_message (encode.message_in_buffer);
        rs_encode (&encode);
        
        switch (ecase)
        {
        case 0:
            corrupt_message1 (encode.message_out_buffer, (n - k) / 2);
            break;

        case 1:
            corrupt_message2 (encode.message_out_buffer, 5, (n - k) / 2);
            break;
        }
        
        memcpy (
          decode.message_in_buffer.memory,
          encode.message_out_buffer.memory,
          encode.field_message_out_length);

        clock_t clock_time1 = clock ();
        for (int i = 0; i < iteration; i++)
        {
            rs_decode (&decode);
          
            if (
              !cmp_blocks_equal (
                encode.message_in_buffer.memory,
                decode.message_out_buffer.memory,
                encode.field_message_in_length)
              && decode.result != DECODE_STATUS_NO_ERRORS_FOUND)
            {
                printf ("Error decoding!!\n");
                poly_print ("original message", encode.message_in_buffer);
                poly_print ("corrupted message", decode.message_in_buffer);
                poly_print ("recovered message", decode.message_out_buffer);
                exit (EXIT_FAILURE);
            };
            
        }

        clock_t clock_time2 = clock ();
        clock_t time_taken = clock_time2 - clock_time1;
        printf (
          "Decoding at n = %d, and k = %d, case type: %s"
          ".\nTotal time taken %.3fs\n",
          n,
          k,
          ecase == 0 ? "Random errors spread even" : "Continous chain of errors",
          (double) time_taken / CLOCKS_PER_SEC);
          fflush(stdout);
        printf (
          "Decoding speed is almost %.3f Mbit/s\n*******************************\n",
          time_taken? ((k * iteration) / (1024.0f * 1024 * time_taken / CLOCKS_PER_SEC) * 8):0);

        rs_close_encoder (&encode);
        rs_close_decoder (&decode);
    }
    return (0);
}